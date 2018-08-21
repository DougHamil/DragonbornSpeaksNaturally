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

        public static string resolveFilePath(string filename) {
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
