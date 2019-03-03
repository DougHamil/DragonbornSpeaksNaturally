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
        private readonly string CONFIG_FILE_NAME = "DragonbornSpeaksNaturally.ini";
        private readonly string COMMAND_FILE_NAME = "DragonbornSpeaksNaturally.ini";

        // NOTE: Relative to SkyrimVR.exe
        private readonly string[] SEARCH_DIRECTORIES = {
            "Data\\Plugins\\Sumwunn\\",
            ""
        };

        private IniData global = null;
        private IniData local = null;
        private IniData merged = null;
        private CommandList consoleCommandList = null;

        public Configuration() { }

        public string Get(string section, string key, string def) {
            string val = getData()[section][key];
            if (val == null)
                return def;
            return val;
        }

        public List<string> GetGoodbyePhrases() {
            string phrases = merged["Dialogue"]["goodbyePhrases"];
            if(phrases != null) {
                List<string> list = new List<string>(phrases.Split(';'));
                list.RemoveAll((str) => str == null || str.Trim() == "");
                return list;
            }
            return new List<string>();
        }

        public CommandList GetConsoleCommandList() {
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

        private IniData getData() {
            if (merged == null) {
                loadLocal();
                loadGlobal();
                merged = new IniData();
                merged.Merge(global);
                merged.Merge(local);
            }

            return merged;
        }

        private void loadGlobal() {
            global = new IniData();
            // global["SpeechRecognition"]["Locale"] = "en-US";
        }

        private void loadLocal() {

            local = loadIniFromFilename(CONFIG_FILE_NAME);
            if (local == null)
                local = new IniData();
        }

        public string resolveFilePath(string filename) {
            foreach (string directory in SEARCH_DIRECTORIES) {
                string filepath = directory + filename;
                if (File.Exists(filepath)) {
                    return filepath;
                }
            }
            return null;
        }

        private IniData loadIniFromFilename(string filename) {
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
