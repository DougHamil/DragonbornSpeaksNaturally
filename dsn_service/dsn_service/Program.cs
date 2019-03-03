using System;
using System.Diagnostics;
using System.Threading;

namespace DSN {
    class Program {
        private static readonly string VERSION = "0.18";

        static void Main(string[] args) {
            try
            {
                Log.Initialize();
                Trace.TraceInformation("DragonbornSpeaksNaturally ({0}) speech recognition service started", VERSION);

                for (int i = 0; i < args.Length; i++)
                {
                    if (args[i].Equals("--encoding") && args.Length >= i + 1)
                    {
                        string encode = args[i+1];

                        // Set encoding of stdin/stdout to the client specified.
                        // This can avoid non-ASCII characters (such as Chinese characters) garbled.
                        Console.InputEncoding = System.Text.Encoding.GetEncoding(encode);
                        Console.OutputEncoding = System.Text.Encoding.GetEncoding(encode);

                        Trace.TraceInformation("Set encoding of stdin/stdout to {0}", encode);
                    }
                }

                Configuration config = new Configuration();
                SkyrimInterop skyrimInterop = new SkyrimInterop(config);
                ExternalInterop externalInterop = new ExternalInterop(skyrimInterop);

                Thread skyrimThread = skyrimInterop.Start();
                Thread externalThread = externalInterop.Start();

                // Skyrim thread will finish when Skyrim closes
                skyrimThread.Join();

                // Cleanup threads
                skyrimInterop.Stop();
                externalInterop.Stop();
                externalThread.Abort();

            } catch (Exception ex) {
                Trace.TraceError(ex.ToString());
            }
        }
    }
}
