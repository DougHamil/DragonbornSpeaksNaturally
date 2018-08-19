using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Speech.Recognition;
using System.Text;
using System.Threading.Tasks;

namespace DSN {
    class FavoritesList : ISpeechRecognitionGrammarProvider {

        private  Dictionary<Grammar, string> commandsByGrammar = new Dictionary<Grammar, string>();
        private string leftHandSuffix;
        private string rightHandSuffix;

        public void Update(string input) {

            if(Configuration.Get("Favorites", "enabled", "1") == "0") {
                return;
            }

            leftHandSuffix = Configuration.Get("Favorites", "equipLeftSuffix", "left");
            rightHandSuffix = Configuration.Get("Favorites", "equipRightSuffix", "right");

            Trace.TraceInformation("Received favorites list: {0}", input);

            string equipPrefix = Configuration.Get("Favorites", "equipPhrasePrefix", "equip");
            commandsByGrammar.Clear();
            string[] itemTokens = input.Split('|');
            foreach(string itemStr in itemTokens) {
                try {
                    string[] tokens = itemStr.Split(',');
                    string itemName = tokens[0];
                    long formId = long.Parse(tokens[1]);
                    long itemId = long.Parse(tokens[2]);
                    bool isSingleHanded = int.Parse(tokens[3]) > 0;
                    int typeId = int.Parse(tokens[4]);

                    string phrase = equipPrefix + " " + Phrases.normalize(itemName);
                    string command = formId + ";" + itemId + ";" + typeId + ";";

                    GrammarBuilder grammarBuilder = new GrammarBuilder(phrase);

                    // Append hand choice if necessary
                    if (isSingleHanded) {
                        Choices handChoice = new Choices(new string[] { leftHandSuffix, rightHandSuffix });
                        grammarBuilder.Append(handChoice, 0, 1); // Optional left/right. When excluded, try to equip to both hands
                    }

                    Grammar grammar = new Grammar(grammarBuilder);
                    grammar.Name = phrase;
                    commandsByGrammar[grammar] = command;
                } catch(Exception ex) {
                    Trace.TraceError("Failed to parse {0} due to exception:\n{1}", itemStr, ex.ToString());
                }
            }

            PrintToTrace();
        }

        public void PrintToTrace() {
            Trace.TraceInformation("Favorites List Phrases:");
            foreach (KeyValuePair<Grammar, string> entry in commandsByGrammar) {
                Trace.TraceInformation("Phrase '{0}' mapped to equip command '{1}'", entry.Key.Name, entry.Value);
            }
        }

        public string GetCommandForResult(RecognitionResult result) {
            Grammar grammar = result.Grammar;
            if (commandsByGrammar.ContainsKey(grammar)) {
                string command = commandsByGrammar[grammar];
                // Determine handedness
                if (result.Text.EndsWith(rightHandSuffix)) {
                    command += "1";
                } else if (result.Text.EndsWith(leftHandSuffix)) {
                    command += "2";
                } else {
                    command += "0";
                }
                return command;
            }

            return null;
        }

        public List<Grammar> GetGrammars() {
            return new List<Grammar>(commandsByGrammar.Keys);
        }
    }
}
