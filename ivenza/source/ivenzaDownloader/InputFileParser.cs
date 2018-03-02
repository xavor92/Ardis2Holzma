using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace ivenzaDownloader
{
    class InputFileParser
    {
        public List<String> getImageIds(String path)
        {
            List<String> ids = new List<string>();
            using (StreamReader reader = new StreamReader(path))
            {
                String line = "";
                while ((line = reader.ReadLine()) != null)
                {
                    ids.Add(line);
                }
            }
            return ids;
        }
    }
}
