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

        private Configuration config;
        private Dictionary<Grammar, string> commandsByGrammar;

        private bool enabled;
        private bool useEquipHandPrefix;

        private string leftHandSuffix;
        private string rightHandSuffix;
        private string bothHandsSuffix;

        private string mainHand;
        private string mainHandId;

        public FavoritesList(Configuration config) {
            this.config = config;
            commandsByGrammar = new Dictionary<Grammar, string>();

            enabled = config.Get("Favorites", "enabled", "1") == "1";
            useEquipHandPrefix = config.Get("Favorites", "useEquipHandPrefix", "0") == "1";

            leftHandSuffix = config.Get("Favorites", "equipLeftSuffix", "left");
            rightHandSuffix = config.Get("Favorites", "equipRightSuffix", "right");
            bothHandsSuffix = config.Get("Favorites", "equipBothSuffix", "both");
            
            mainHand = config.Get("Favorites", "mainHand", "none");

            // Determine the main hand used when user didn't ask for a specific hand.
            // 
            // If an initializer is used and the key name conflicts (such as bothHandsSuffix == "both"),
            // an System.ArgumentException will be thrown. So assigning values one by one is a safer way.
            var mainHandMap = new Dictionary<string, string>();
            mainHandMap[bothHandsSuffix] = "0";
            mainHandMap[rightHandSuffix] = "1";
            mainHandMap[leftHandSuffix] = "2";

            // Comment of `mainHand` in `DragonbornSpeaksNaturally.SAMPLE.ini` said:
            // > Valid values are "right", "left", "both"
            // We should keep the compatibility to prevent user confusion.
            mainHandMap["both"] = "0";
            mainHandMap["right"] = "1";
            mainHandMap["left"] = "2";

            if (mainHandMap.ContainsKey(mainHand))
            {
                mainHandId = mainHandMap[mainHand];
            }
            else {
                // User does not specify the main hand. Equipped with both hands by default.
                mainHandId = "0";
            }
        }

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
            Choices handChoice = new Choices(new string[] { bothHandsSuffix, leftHandSuffix, rightHandSuffix });
            GrammarBuilder grammarBuilder = new GrammarBuilder();

            // Append hand choice prefix
            if (isSingleHanded && useEquipHandPrefix)
            {
                // Optional left/right. When excluded, try to equip to both hands
                grammarBuilder.Append(handChoice, 0, 1);
            }

            grammarBuilder.Append(phrase);

            // Append hand choice suffix
            if (isSingleHanded && !useEquipHandPrefix)
            {
                // Optional left/right. When excluded, try to equip to both hands
                grammarBuilder.Append(handChoice, 0, 1);
            }

            Grammar grammar = new Grammar(grammarBuilder);
            grammar.Name = phrase;
            commandsByGrammar[grammar] = command;
        }

        // Locates and loads item name replacement maps
        // Returns dynamic map/dictionary or null when the replacement map files cannot be located
        public dynamic LoadItemNameMap()
        {
            string filepath = config.resolveFilePath("item-name-map.json");
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
            if(!enabled) {
                return;
            }

            var firstEquipmentOfType = new Dictionary<string, string> { };
           
            dynamic itemNameMap = LoadItemNameMap();

            string equipPrefix = config.Get("Favorites", "equipPhrasePrefix", "equip");
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

        private bool hasPrefixOrSuffix(string text, string prefixOrSuffix) {
            //
            // NOTICE: Some languages (such as Chinese) do not use spaces to separate words.
            // So the code such as `result.Text.Split(' ').Last()` will not work for them.
            // Be aware of this when changing the code below.
            //
            return useEquipHandPrefix ? text.StartsWith(prefixOrSuffix) : text.EndsWith(prefixOrSuffix);
        }

        public string GetCommandForResult(RecognitionResult result) {
            Grammar grammar = result.Grammar;
            if (commandsByGrammar.ContainsKey(grammar)) {
                string command = commandsByGrammar[grammar];

                // Determine handedness
                //
                // NOTICE: Some languages (such as Chinese) do not use spaces to separate words.
                // So the code such as `result.Text.Split(' ').Last()` will not work for them.
                // Be aware of this when changing the code below.
                //
                if (hasPrefixOrSuffix(result.Text, bothHandsSuffix))
                {
                    command += "0";
                }
                else if(hasPrefixOrSuffix(result.Text, rightHandSuffix))
                {
                    command += "1";
                }
                else if (hasPrefixOrSuffix(result.Text, leftHandSuffix))
                {
                    command += "2";
                }
                else
                {
                    // The user didn't ask for a specific hand, supply a default
                    command += mainHandId;
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
