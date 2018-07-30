using IniParser;
using IniParser.Model;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DSN {

    public class ReplPhrase
    {
        public string pattern = "";
        public string replacement = "";

        public ReplPhrase(string input)
        {
            string[] tokens = input.Split(new string[] {"-->"},2,StringSplitOptions.None);
            pattern = tokens[0];
            if (tokens.Length > 1)
                replacement = tokens[1];

            Trace.TraceInformation("replacePhrase: " + pattern + " with " + replacement);
        }
    }

    class Configuration {
        private static readonly string CONFIG_FILE_NAME = "DragonbornSpeaksNaturally.ini";
        private static readonly string COMMAND_FILE_NAME = "DragonbornSpeaksNaturally.ini";

        // NOTE: Relative to SkyrimVR.exe
        private static readonly string[] SEARCH_DIRECTORIES = {
            "Data\\Plugins\\Sumwunn\\",
            ""
        };

        private static IniData global = null;
        private static IniData local = null;
        private static IniData merged = null;
        private static CommandList consoleCommandList = null;
        private static List<ReplPhrase> replacePhrases = null;
        private static List<ReplPhrase> replaceRegExPhrases = null;

        private Configuration() { }

        public static string Get(string section, string key, string def) {
            string val = getData()[section][key];
            if (val == null)
                return def;
            return val;
        }

        public static List<string> GetGoodbyePhrases() {
            string phrases = merged["Dialogue"]["goodbyePhrases"];
            if(phrases != null) {
                List<string> list = new List<string>(phrases.Split(';'));
                list.RemoveAll((str) => str == null || str.Trim() == "");
                return list;
            }
            return new List<string>();
        }

        public static void LoadReplacePhrases()
        {
            replacePhrases = new List<ReplPhrase>();
            replaceRegExPhrases = new List<ReplPhrase>();

            KeyDataCollection keyCollection = merged["Favorites"];

            foreach (var keyData in keyCollection)
            {
                if (keyData.KeyName.StartsWith("ReplacePhrase"))
                {
                    replacePhrases.Add(new ReplPhrase(keyData.Value));
                }
                else if (keyData.KeyName.StartsWith("ReplaceRegExPhrase"))
                {
                    replaceRegExPhrases.Add(new ReplPhrase(keyData.Value));
                }
            }
        }

        public static List<ReplPhrase> GetReplacePhrases()
        {
            getData();
            return replacePhrases;
        }

        public static List<ReplPhrase> GetReplaceRegExPhrases()
        {
            getData();
            return replaceRegExPhrases;
        }

        public static CommandList GetConsoleCommandList() {
            if(consoleCommandList != null)
                return consoleCommandList;

            IniData commandData = loadIniFromFilename(COMMAND_FILE_NAME);
            if(commandData != null) {
                consoleCommandList = CommandList.FromIniSection(commandData, "ConsoleCommands");
            } else {
                consoleCommandList = new CommandList();
            }

            Trace.TraceInformation("Loaded Console Commands:");
            consoleCommandList.PrintToTrace();

            return consoleCommandList;
        }

        private static IniData getData() {
            if (merged == null) {
                loadLocal();
                loadGlobal();
                merged = new IniData();
                merged.Merge(global);
                merged.Merge(local);
                LoadReplacePhrases();
            }

            return merged;
        }

        private static void loadGlobal() {
            global = new IniData();
            // global["SpeechRecognition"]["Locale"] = "en-US";
        }

        private static void loadLocal() {

            local = loadIniFromFilename(CONFIG_FILE_NAME);
            if (local == null)
                local = new IniData();
        }

        private static string resolveFilePath(string filename) {
            foreach (string directory in SEARCH_DIRECTORIES) {
                string filepath = directory + filename;
                if (File.Exists(filepath)) {
                    return filepath;
                }
            }
            return null;
        }

        private static IniData loadIniFromFilename(string filename) {
            string filepath = resolveFilePath(filename);
            if (filepath != null) {
                Trace.TraceInformation("Loading ini from path " + filepath);
                try {
                    var parser = new FileIniDataParser();
                    return parser.ReadFile(filepath);
                } catch (Exception ex) {
                    Trace.TraceError("Failed to load ini file at " + filepath);
                    Trace.TraceError(ex.ToString());
                }
            }
            return null;
        }
    }
}
