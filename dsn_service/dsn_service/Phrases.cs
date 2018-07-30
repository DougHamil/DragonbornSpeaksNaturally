using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Diagnostics;

namespace DSN {
    class Phrases {
        public static string normalize(string phrase)
        {
            Regex regex = new Regex("[ ]{2,}", RegexOptions.None);
            phrase = regex.Replace(phrase, " ");

            List<ReplPhrase> replacePhrases = Configuration.GetReplacePhrases();
            List<ReplPhrase> replaceRegExPhrases = Configuration.GetReplaceRegExPhrases();

            foreach (ReplPhrase replacePhrase in replaceRegExPhrases)
            {
                phrase = Regex.Replace(phrase, replacePhrase.pattern, replacePhrase.replacement);
            }

            foreach (ReplPhrase replacePhrase in replacePhrases)
            {
                string pattern = @"\b(" + replacePhrase.pattern + @")\b";
                // Trace.TraceInformation("Trying to replace " + pattern + " with " + replacePhrase.replacement);
                phrase = Regex.Replace(phrase, pattern, replacePhrase.replacement);
            }

            phrase = Regex.Replace(phrase, "[\"']", string.Empty);
            phrase = phrase.Replace('-', ' ');
            return phrase.Trim();

        }
    }
}
