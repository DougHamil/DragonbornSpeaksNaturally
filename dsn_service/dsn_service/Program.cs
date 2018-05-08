using System;
using System.Diagnostics;
using System.Threading;

namespace DSN {
    class Program {
        private static readonly string VERSION = "0.14";

        static void Main(string[] args) {
            try {
                Log.Initialize();
                Trace.TraceInformation("DragonbornSpeaksNaturally ({0}) speech recognition service started", VERSION);
                Thread skyrimThread = SkyrimInterop.Start();
                Thread externalThread = ExternalInterop.Start();

                // Skyrim thread will finish when Skyrim closes
                skyrimThread.Join();

                // Cleanup threads
                SkyrimInterop.Stop();
                ExternalInterop.Stop();
                externalThread.Abort();

            } catch (Exception ex) {
                Trace.TraceError(ex.ToString());
            }
        }
    }
}
