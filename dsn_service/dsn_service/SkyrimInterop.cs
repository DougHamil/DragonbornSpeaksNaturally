using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Speech.Recognition;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace DSN {
    class SkyrimInterop {

        private static System.Object dialogueLock = new System.Object();
        private static DialogueList currentDialogue = null;
        private static FavoritesList favoritesList = null;
        private static SpeechRecognitionManager recognizer;
        private static Thread submissionThread;
        private static BlockingCollection<string> commandQueue;

        public static Thread Start() {
            try {
                favoritesList = new FavoritesList();
                commandQueue = new BlockingCollection<string>();
                recognizer = new SpeechRecognitionManager();
                recognizer.OnDialogueLineRecognized += Recognizer_OnDialogueLineRecognized;

                // Start in command-mode
                recognizer.StartSpeechRecognition(false, Configuration.GetConsoleCommandList(), favoritesList);

                Thread listenThread = new Thread(ListenForInput);
                submissionThread = new Thread(SubmitCommands);
                submissionThread.Start();
                listenThread.Start();
                return listenThread;
            }
            catch (Exception ex) {
                Trace.TraceError("Failed to initialize speech recognition due to error:");
                Trace.TraceError(ex.ToString());
            }

            return null;
        }

        public static void Stop() {
            submissionThread.Abort();
            recognizer.Stop();
        }

        public static void SubmitCommand(string command) {
            commandQueue.Add(sanitize(command));
        }

        private static string sanitize(string command) {
            command = command.Trim();
            return command.Replace("\r", "");
        }

        private static void SubmitCommands() {
            while(true) {
                string command = commandQueue.Take();
                Trace.TraceInformation("Sending command: {0}", command);
                Console.Write(command+"\n");
            }
        }

        private static void ListenForInput() {
            try {
                while (true) {
                    string input = Console.ReadLine();

                    // input will be null when Skyrim is terminated
                    if (input == null) {
                        Trace.TraceInformation("Skyrim is terminated, recognition service will quit.");
                        break;
                    }

                    Trace.TraceInformation("Received command: {0}", input);

                    string[] tokens = input.Split('|');
                    string command = tokens[0];
                    if (command.Equals("START_DIALOGUE")) {
                        lock (dialogueLock) {
                            currentDialogue = DialogueList.Parse(string.Join("|", tokens, 1, tokens.Length - 1));
                        }
                        // Switch to dialogue mode
                        recognizer.StartSpeechRecognition(true, currentDialogue);
                    } else if (command.Equals("STOP_DIALOGUE")) {
                        // Switch to command mode
                        recognizer.StartSpeechRecognition(false, Configuration.GetConsoleCommandList(), favoritesList);
                        lock (dialogueLock) {
                            currentDialogue = null;
                        }
                    } else if (command.Equals("FAVORITES")) {
                        favoritesList.Update(string.Join("|", tokens, 1, tokens.Length - 1));
                        if(currentDialogue == null) {
                            recognizer.StartSpeechRecognition(false, Configuration.GetConsoleCommandList(), favoritesList);
                        }
                    }
                }
            } catch (Exception ex) {
                Trace.TraceError(ex.ToString());
            }
        }

        private static void Recognizer_OnDialogueLineRecognized(RecognitionResult result) {
            string line = result.Text;

            lock (dialogueLock) {
                if (currentDialogue != null) {
                    int idx = currentDialogue.GetLineIndex(result.Grammar);
                    if (idx != -1)
                        SubmitCommand("DIALOGUE|" + currentDialogue.id + "|" + idx);
                } else {
                    string command = favoritesList.GetCommandForResult(result);
                    if(command != null) {
                        SubmitCommand("EQUIP|" + command);
                    } else {
                        command = Configuration.GetConsoleCommandList().GetCommandForPhrase(result.Grammar);
                        if (command != null) {
                            SubmitCommand("COMMAND|" + command);
                        }
                    }
                }
            }
        }
    }
}
