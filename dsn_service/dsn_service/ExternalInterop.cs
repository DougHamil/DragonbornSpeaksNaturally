using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace DSN {
    class ExternalInterop {

        private static readonly HashSet<string> BATCH_FILENAMES = new HashSet<string>() { "wotv", "ivrqs" };
        private static readonly long FILE_CHANGE_DEBOUNCE_TIME_TICKS = 10000 * 200; // 200 ms

        private static FileSystemWatcher watcher;
        private static DateTime lastChangeDt = DateTime.Now;


        public static Thread Start() {
            Thread thread = new Thread(ListenForInput);
            thread.Start();
            return thread;
        }

        public static void Stop() {
            watcher.EnableRaisingEvents = false;
        }

        private static void ListenForInput() {
            try {
                string exepath = System.IO.Path.GetDirectoryName(System.Diagnostics.Process.GetCurrentProcess().MainModule.FileName);
                Trace.TraceInformation(exepath);
                watcher = new FileSystemWatcher();
                watcher.Path = Directory.GetCurrentDirectory();
                watcher.Changed += Watcher_Changed;
                watcher.NotifyFilter = NotifyFilters.LastWrite | NotifyFilters.CreationTime;
                Trace.TraceInformation("Watching for batch files at {0}", watcher.Path);
                watcher.EnableRaisingEvents = true;
            }
            catch(Exception ex) {
                Trace.TraceError("Failed to watch for batch files");
                Trace.TraceError(ex.ToString());
            }
        }

        private static void Watcher_Changed(object sender, FileSystemEventArgs e) {
            if(e.ChangeType == WatcherChangeTypes.Changed || e.ChangeType == WatcherChangeTypes.Created) {
                string filename = e.Name.ToLower();
                foreach(string watchedFilename in BATCH_FILENAMES) {
                    if (filename.Equals(watchedFilename)) {
                        DateTime now = DateTime.Now;
                        if (now.Ticks - lastChangeDt.Ticks >= FILE_CHANGE_DEBOUNCE_TIME_TICKS) {
                            SkyrimInterop.SubmitCommand("COMMAND|bat " + watchedFilename);
                        }
                        lastChangeDt = now;
                    }
                }
            }
        }
    }
}
