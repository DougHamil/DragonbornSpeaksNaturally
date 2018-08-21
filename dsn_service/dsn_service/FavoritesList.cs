using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Speech.Recognition;
using System.Text;
using System.Threading.Tasks;
using System.Web.Script.Serialization;
using System.IO;

namespace DSN {
    class FavoritesList : ISpeechRecognitionGrammarProvider {

        private  Dictionary<Grammar, string> commandsByGrammar = new Dictionary<Grammar, string>();
        private string leftHandSuffix = Configuration.Get("Favorites", "equipLeftSuffix", "left");
        private string rightHandSuffix = Configuration.Get("Favorites", "equipRightSuffix", "right");

        private HashSet<string> knownEquipmentTypes = new HashSet<string>
        {
            "daggar", "mace", "sword", "axe", "battleaxe", "greatsword", "warhammer", "bow", "crossbow", "shield"
        };

        public string ProbableEquipmentType(string itemName)
        {
            var tokens = new HashSet<string> { };

            foreach (string token in itemName.Split(" [](),|:".ToCharArray()))
            {
                tokens.Add(token.ToLower());
            }

            var intersection = tokens.Intersect(knownEquipmentTypes);
            if (intersection.Count() == 0)
                return null;
            else
                return intersection.First();
        }

        public void BuildAndAddGrammar(string phrase, string command, bool isSingleHanded)
        {
            GrammarBuilder grammarBuilder = new GrammarBuilder(phrase);

            // Append hand choice if necessary
            if (isSingleHanded)
            {
                Choices handChoice = new Choices(new string[] { leftHandSuffix, rightHandSuffix });
                grammarBuilder.Append(handChoice, 0, 1); // Optional left/right. When excluded, try to equip to both hands
            }

            Grammar grammar = new Grammar(grammarBuilder);
            grammar.Name = phrase;
            commandsByGrammar[grammar] = command;
        }

        // Locates and loads item name replacement maps
        // Returns dynamic map/dictionary or null when the replacement map files cannot be located
        public dynamic LoadItemNameMap()
        {
            string filepath = Configuration.resolveFilePath("item-name-map.json");
            if(File.Exists(filepath))
            {
                return LoadItemNameMap(filepath);
            }
            return null;
        }
        
        // Returns a map/dictionary or throws exception when the file cannot be opened/read
        public dynamic LoadItemNameMap(string path)
        {
            var json = System.IO.File.ReadAllText(path);
            JavaScriptSerializer jsonSerializer = new JavaScriptSerializer();
            return jsonSerializer.Deserialize<dynamic>(json);
        }

        public string MaybeReplaceItemName(dynamic nameMap, string itemName)
        {
            if (nameMap == null)
                return itemName;

            try
            {
                return nameMap[itemName];
            }
            catch (KeyNotFoundException)
            {
                return itemName;
            }
        }

 

        public void Update(string input) {
            if(Configuration.Get("Favorites", "enabled", "1") == "0") {
                return;
            }

            var firstEquipmentOfType = new Dictionary<string, string> { };
           
            dynamic itemNameMap = LoadItemNameMap();

            Trace.TraceInformation("Received favorites list: {0}", input);

            string equipPrefix = Configuration.Get("Favorites", "equipPhrasePrefix", "equip");
            commandsByGrammar.Clear();
            string[] itemTokens = input.Split('|');
            foreach(string itemStr in itemTokens) {
                try
                {
                    string[] tokens = itemStr.Split(',');
                    string itemName = tokens[0];
                    long formId = long.Parse(tokens[1]);
                    long itemId = long.Parse(tokens[2]);
                    bool isSingleHanded = int.Parse(tokens[3]) > 0;
                    int typeId = int.Parse(tokens[4]);

                    itemName = MaybeReplaceItemName(itemNameMap, itemName);

                    string phrase = equipPrefix + " " + Phrases.normalize(itemName);
                    string command = formId + ";" + itemId + ";" + typeId + ";";

                    BuildAndAddGrammar(phrase, command, isSingleHanded);

                    // Are we looking at an equipment of some sort?
                    // Record the first item of a specific weapon type
                    string equipmentType = ProbableEquipmentType(itemName);
                    if(equipmentType != null && firstEquipmentOfType.ContainsKey(equipmentType) == false)
                    {
                        BuildAndAddGrammar(equipPrefix + " " + equipmentType, command, isSingleHanded);
                    }
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
