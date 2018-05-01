using System;
using System.Collections.Generic;
using System.Linq;
using System.Speech.Recognition;
using System.Diagnostics;
using System.Globalization;

namespace DSN {
    class SpeechRecognitionManager {

        public delegate void DialogueLineRecognitionHandler(RecognitionResult result);
        public event DialogueLineRecognitionHandler OnDialogueLineRecognized;

        private SpeechRecognitionEngine DSN;
        private float dialogueMinimumConfidence = 0.5f; // Dialogue can be more generous in the min confidence because phrases are usually longer and more distinct amongst themselves
        private float commandMinimumConfidence = 0.7f;
        private bool isDialogueMode = false;

        public SpeechRecognitionManager() {
            string locale = Configuration.Get("SpeechRecognition", "Locale", CultureInfo.InstalledUICulture.Name);
            dialogueMinimumConfidence = float.Parse(Configuration.Get("SpeechRecognition", "dialogueMinConfidence", "0.5"), CultureInfo.InvariantCulture);
            commandMinimumConfidence = float.Parse(Configuration.Get("SpeechRecognition", "commandMinConfidence", "0.7"), CultureInfo.InvariantCulture);

            Trace.TraceInformation("Locale: {0}\nDialogueConfidence: {1}\nCommandConfidence: {2}", locale, dialogueMinimumConfidence, commandMinimumConfidence);

            this.DSN = new SpeechRecognitionEngine(new CultureInfo(locale));
            this.DSN.UpdateRecognizerSetting("CFGConfidenceRejectionThreshold", 10); // Range is 0-100
            this.DSN.SetInputToDefaultAudioDevice();
            this.DSN.EndSilenceTimeoutAmbiguous = TimeSpan.FromMilliseconds(250);
            this.DSN.AudioSignalProblemOccurred += DSN_AudioSignalProblemOccurred;
            this.DSN.SpeechRecognized += DSN_SpeechRecognized;
            this.DSN.SpeechRecognitionRejected += DSN_SpeechRecognitionRejected;
        }


        private void DSN_AudioSignalProblemOccurred(object sender, AudioSignalProblemOccurredEventArgs e) {
            if(Configuration.Get("SpeechRecognition", "bLogAudioSignalIssues", "0") == "1") {
                Trace.TraceInformation("Audio signal problem occurred during speech recognition:");
                Trace.TraceInformation(e.AudioSignalProblem.ToString());
            }
        }

        public void StopRecognition() {
            this.DSN.RecognizeAsyncCancel();
        }

        public void StartSpeechRecognition(bool isDialogueMode, params ISpeechRecognitionGrammarProvider[] grammarProviders) {
            try {
                this.isDialogueMode = isDialogueMode;
                this.DSN.RecognizeAsyncCancel();
                List<Grammar> allGrammars = grammarProviders.SelectMany((x) => x.GetGrammars()).ToList();
                setGrammar(allGrammars);
            } catch (Exception e) {
                Trace.TraceError("Failed to start new phrase recognition due to exception");
                Trace.TraceError(e.ToString());
            }
        }

        private void setGrammar(List<Grammar> grammars) {
            this.DSN.RequestRecognizerUpdate();
            this.DSN.UnloadAllGrammars();
            foreach (Grammar grammar in grammars) {
                this.DSN.LoadGrammarAsync(grammar);
            }
            // Error is thrown if no grammars are loaded
            if(grammars.Count > 0)
                this.DSN.RecognizeAsync(RecognizeMode.Multiple);
        }

        private void DSN_SpeechRecognitionRejected(object sender, SpeechRecognitionRejectedEventArgs e) {
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
