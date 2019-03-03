using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Speech.Recognition;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace DSN
{
    class ConsoleInput
    {

        private BlockingCollection<string> inputQueue = new BlockingCollection<string>();
        private Thread inputThread = null;
        bool isInputTerminated = false;

        public void Start()
        {
            inputThread = new Thread(ReadLineFromConsole);
            inputThread.Start();
        }

        private void ReadLineFromConsole()
        {
            while (true)
            {
                string input = Console.ReadLine();

                // input will be null when Skyrim terminated (stdin closed)
                if (input == null)
                {
                    isInputTerminated = true;
                    Trace.TraceInformation("Skyrim is terminated, recognition service will quit.");

                    // Notify the SkyrimInterop thread to exit
                    inputQueue.Add(null);

                    break;
                }

                inputQueue.Add(input);
            }
        }

        public bool IsInputTerminated() {
            return isInputTerminated;
        }

        public void WriteLine(string line) {
            inputQueue.Add(line);
        }

        public string ReadLine() {
            return inputQueue.Take();
        }
    }
}
