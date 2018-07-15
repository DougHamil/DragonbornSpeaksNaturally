using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DSN {
    class Log {

        private static readonly string ERROR_LOG_FILE = "DragonbornSpeaksNaturally.log";

        public static void Initialize() {
            string logFilePath = System.Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments);
            logFilePath += "\\DragonbornSpeaksNaturally";
            System.IO.Directory.CreateDirectory(logFilePath);
            logFilePath += "\\"+ERROR_LOG_FILE;
            try {
                var listener = new TextWriterTraceListener(logFilePath);
                listener.TraceOutputOptions = TraceOptions.DateTime;
                Trace.AutoFlush = true;
                Trace.Listeners.Add(listener);
            }
            catch(Exception ex) {
                Console.Error.WriteLine("Failed to create log file at " + logFilePath + ": {0}", ex.ToString());
            }
    
        }
    }
}
