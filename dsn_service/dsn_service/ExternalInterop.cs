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

        SkyrimInterop skyrimInterop;

        private readonly HashSet<string> BATCH_FILENAMES = new HashSet<string>() { "wotv", "ivrqs" };
        private readonly long FILE_CHANGE_DEBOUNCE_TIME_TICKS = 10000 * 200; // 200 ms

        private FileSystemWatcher watcher;
        private DateTime lastChangeDt = DateTime.Now;


        public ExternalInterop(SkyrimInterop skyrimInterop) {
            this.skyrimInterop = skyrimInterop;
        }

        public Thread Start() {
            Thread thread = new Thread(ListenForInput);
            thread.Start();
            return thread;
        }

        public void Stop() {
            watcher.EnableRaisingEvents = false;
        }

        private void ListenForInput() {
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

        private void Watcher_Changed(object sender, FileSystemEventArgs e) {
            if(e.ChangeType == WatcherChangeTypes.Changed || e.ChangeType == WatcherChangeTypes.Created) {
                string filename = e.Name.ToLower();
                foreach(string watchedFilename in BATCH_FILENAMES) {
                    if (filename.Equals(watchedFilename)) {
                        DateTime now = DateTime.Now;
                        if (now.Ticks - lastChangeDt.Ticks >= FILE_CHANGE_DEBOUNCE_TIME_TICKS) {
                            skyrimInterop.SubmitCommand("COMMAND|bat " + watchedFilename);
                        }
                        lastChangeDt = now;
                    }
                }
            }
        }
    }
}
