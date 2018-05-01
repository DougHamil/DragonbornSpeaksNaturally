using IniParser.Model;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Speech.Recognition;
using System.Text;
using System.Threading.Tasks;

namespace DSN {
    class CommandList : ISpeechRecognitionGrammarProvider {

        public static CommandList FromIniSection(IniData ini, string sectionName) {
            KeyDataCollection sectionData = ini.Sections[sectionName];
            CommandList list = new CommandList();
            if(sectionData != null) {
                foreach(KeyData key in sectionData) {
                    Grammar grammar = new Grammar(new GrammarBuilder(key.KeyName));
                    list.commandsByPhrase[grammar] = key.Value.Trim();
                }
            }
            return list;
        }

        public Dictionary<Grammar, string> commandsByPhrase = new Dictionary<Grammar, string>();

        public string GetCommandForPhrase(Grammar grammar) {
            if (commandsByPhrase.ContainsKey(grammar))
                return commandsByPhrase[grammar];
            return null;
        }

        public void PrintToTrace() {
            Trace.TraceInformation("Command List Phrases:");
            foreach (KeyValuePair<Grammar, string> entry in commandsByPhrase) {
                Trace.TraceInformation("Phrase '{0}' mapped to commands '{1}'", entry.Key, entry.Value);
            }
        }

        public List<Grammar> GetGrammars() {
            return commandsByPhrase.Keys.ToList();
        }
    }
}
