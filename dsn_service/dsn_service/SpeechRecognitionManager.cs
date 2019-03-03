using System;
using System.Collections.Generic;
using System.Linq;
using System.Speech.Recognition;
using System.Diagnostics;
using System.Globalization;
using System.Threading;

namespace DSN {
    class SpeechRecognitionManager {

        private const long STATUS_STOPPED        = 0; // not in recognizing
        private const long STATUS_RECOGNIZING    = 1; // in recognizing
        private const long STATUS_WAITING_DEVICE = 2; // waiting for record device

        public delegate void DialogueLineRecognitionHandler(RecognitionResult result);
        public event DialogueLineRecognitionHandler OnDialogueLineRecognized;

        private long recognitionStatus = STATUS_STOPPED; // Need thread safety.
        private readonly SpeechRecognitionEngine DSN;    // Need thread safety.
        private float dialogueMinimumConfidence = 0.5f;  // Dialogue can be more generous in the min confidence because phrases are usually longer and more distinct amongst themselves
        private float commandMinimumConfidence = 0.7f;
        private bool isDialogueMode = false;
        private ISpeechRecognitionGrammarProvider[] grammarProviders;

        private Thread waitingDeviceThread;
        private Configuration config;

        public SpeechRecognitionManager(Configuration config) {
            this.config = config;

            string locale = config.Get("SpeechRecognition", "Locale", CultureInfo.InstalledUICulture.Name);
            dialogueMinimumConfidence = float.Parse(config.Get("SpeechRecognition", "dialogueMinConfidence", "0.5"), CultureInfo.InvariantCulture);
            commandMinimumConfidence = float.Parse(config.Get("SpeechRecognition", "commandMinConfidence", "0.7"), CultureInfo.InvariantCulture);

            Trace.TraceInformation("Locale: {0}\nDialogueConfidence: {1}\nCommandConfidence: {2}", locale, dialogueMinimumConfidence, commandMinimumConfidence);

            this.DSN = new SpeechRecognitionEngine(new CultureInfo(locale));
            this.DSN.UpdateRecognizerSetting("CFGConfidenceRejectionThreshold", 10); // Range is 0-100
            this.DSN.EndSilenceTimeoutAmbiguous = TimeSpan.FromMilliseconds(250);
            this.DSN.AudioStateChanged += DSN_AudioStateChanged;
            this.DSN.AudioSignalProblemOccurred += DSN_AudioSignalProblemOccurred;
            this.DSN.SpeechRecognized += DSN_SpeechRecognized;
            this.DSN.SpeechRecognitionRejected += DSN_SpeechRecognitionRejected;

            WaitRecordingDeviceNonBlocking();
        }

        public void Stop() {
            if (waitingDeviceThread != null) {
                waitingDeviceThread.Abort();
            }
        }

        private void WaitRecordingDeviceNonBlocking() {
            // Waiting recording device in a new thread to avoid blocking
            waitingDeviceThread = new Thread(DoWaitRecordingDevice);
            waitingDeviceThread.Start();
        }

        private void DoWaitRecordingDevice() {
            lock (DSN) {
                StopRecognition();

                // Thread-safe:recognitionStatus = STATUS_WAITING_DEVICE
                Interlocked.Exchange(ref recognitionStatus, STATUS_WAITING_DEVICE);

                // Retry until there is an audio input device available
                bool logWaitingRecDev = false;
                for (; ; ) {
                    try {
                        this.DSN.SetInputToDefaultAudioDevice();
                        Trace.TraceInformation("The recording device is ready.");
                        break;
                    } catch (System.InvalidOperationException) {
                        if (!logWaitingRecDev) {
                            Trace.TraceInformation("Waiting for available recording device...");
                            logWaitingRecDev = true;
                        }
                        System.Threading.Thread.Sleep(1000);
                    }
                }

                // Thread-safe:recognitionStatus = STATUS_STOPPED
                Interlocked.Exchange(ref recognitionStatus, STATUS_STOPPED);

                // Restart recognition
                if (grammarProviders != null && grammarProviders.Length > 0) {
                    StartSpeechRecognition(isDialogueMode, grammarProviders);
                }
            }

            waitingDeviceThread = null;
        }

        private void DSN_AudioStateChanged(object sender, AudioStateChangedEventArgs e) {
            if (config.Get("SpeechRecognition", "bLogAudioSignalIssues", "0") == "1") {
                Trace.TraceInformation("Audio state changed: {0}", e.AudioState.ToString());
            }

            // Thread-safe: if (e.AudioState == AudioState.Stopped && recognitionStatus == STATUS_RECOGNIZING)
            if (e.AudioState == AudioState.Stopped && Interlocked.Read(ref recognitionStatus) == STATUS_RECOGNIZING) {
                Trace.TraceInformation("The recording device is not available.");
                WaitRecordingDeviceNonBlocking();
            }
        }

        private void DSN_AudioSignalProblemOccurred(object sender, AudioSignalProblemOccurredEventArgs e) {
            if(config.Get("SpeechRecognition", "bLogAudioSignalIssues", "0") == "1") {
                Trace.TraceInformation("Audio signal problem occurred during speech recognition: {0}", e.AudioSignalProblem.ToString());
            }
        }

        private void StopRecognition() {
            // Thread-safe: if (recognitionStatus == STATUS_RECOGNIZING) { recognitionStatus = STATUS_STOPPED; ... }
            if (Interlocked.CompareExchange(ref recognitionStatus, STATUS_STOPPED, STATUS_RECOGNIZING) == STATUS_RECOGNIZING) {
                this.DSN.RecognizeAsyncCancel();
            }
        }

        public void StartSpeechRecognition(bool isDialogueMode, params ISpeechRecognitionGrammarProvider[] grammarProviders) {
            try {
                this.isDialogueMode = isDialogueMode;
                this.grammarProviders = grammarProviders;

                // Thread-safe: if (recognitionStatus == STATUS_WAITING_DEVICE)
                if (Interlocked.Read(ref recognitionStatus) == STATUS_WAITING_DEVICE) {
                    // Avoid blocking and the program cannot quit when Skyrim is terminated
                    Trace.TraceInformation("Recognition not start because waiting for recording device");
                    return;
                }

                lock (DSN) {
                    StopRecognition(); // Cancel previous recognition

                    List<Grammar> allGrammars = grammarProviders.SelectMany((x) => x.GetGrammars()).ToList();
                    // Error is thrown if no grammars are loaded
                    if (allGrammars.Count > 0) {
                        SetGrammar(allGrammars);
                        this.DSN.RecognizeAsync(RecognizeMode.Multiple);
                        // Thread-safe: recognitionStatus = STATUS_RECOGNIZING
                        Interlocked.Exchange(ref recognitionStatus, STATUS_RECOGNIZING);
                    }
                }
            } catch (Exception e) {
                Trace.TraceError("Failed to start new phrase recognition due to exception");
                Trace.TraceError(e.ToString());
            }
        }

        private void SetGrammar(List<Grammar> grammars) {
            this.DSN.RequestRecognizerUpdate();
            this.DSN.UnloadAllGrammars();
            foreach (Grammar grammar in grammars) {
                this.DSN.LoadGrammarAsync(grammar);
            }
        }

        private void DSN_SpeechRecognitionRejected(object sender, SpeechRecognitionRejectedEventArgs e) {
            // nothing to do
        }

        private void DSN_SpeechRecognized(object sender, SpeechRecognizedEventArgs e) {
            float minConfidence = isDialogueMode ? dialogueMinimumConfidence : commandMinimumConfidence;
            if (e.Result.Confidence >= minConfidence) {
                Trace.TraceInformation("Recognized phrase '{0}' (Confidence: {1})", e.Result.Text, e.Result.Confidence);
                OnDialogueLineRecognized?.Invoke(e.Result);
            } else {
                Trace.TraceInformation("Recognized phrase '{0}' but ignored because confidence was too low (Confidence: {1})", e.Result.Text, e.Result.Confidence);
            }
        }
    }
}
