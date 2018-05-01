using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace DSN {
    class Phrases {
        public static string normalize(string phrase) {
            phrase = Regex.Replace(phrase, @"\(.*?\)", string.Empty);
            phrase = Regex.Replace(phrase, "[\"']", string.Empty);
            phrase = phrase.Replace('-', ' ');
            RegexOptions options = RegexOptions.None;
            Regex regex = new Regex("[ ]{2,}", options);
            phrase = regex.Replace(phrase, " ");
            return phrase.Trim();
        }
    }
}
