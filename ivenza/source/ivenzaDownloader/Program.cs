using System;
using System.Collections.Generic;
using System.Configuration;
using System.Text;
using System.Collections.Specialized;
using System.IO;
using System.Net;
using System.Text.RegularExpressions;

namespace ivenzaDownloader
{
    class Program
    {
        static void Main(string[] args)
        {

            if (args.Length == 0)
            {
                Console.WriteLine();
                Console.WriteLine(System.Diagnostics.Process.GetCurrentProcess().MainModule.FileName + " -d \"C:\\path\\to\\input\\file.dl\" -f \"C:\\path\\to\\output\\folder\\\"");
                return;
            }

            try
            {
                Settings settings = new Settings();
                LoadSettingsFromConfig(settings);
                LoadParameterFromConsole(settings, args);
                createOutputDirectoryIfNotPresent(settings.OutputPath);

                //Lädt die Bilder-Ids aus der Textdatei
                List<string> imagesIds = new InputFileParser().getImageIds(settings.InputFile);
                
                //Erstellt einen neuen WebClient für den Download
                CookieAwareWebClient client = null;
                client = createWebClientAndLogin(settings);

                //Versucht jedes Bild runterzuladen
                for (int i = 0; i < imagesIds.Count; i++)
                {
                    Console.WriteLine("Lade Bild {0} von {1}", i + 1, imagesIds.Count);
                    try
                    {
                        string imageUrl = settings.DownloadTemplate.Replace("{0}", imagesIds[i]);
                        string outputFile = Path.Combine(settings.OutputPath, imagesIds[i] + ".png");
                        client.DownloadFile(imageUrl, outputFile);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine("Fehler beim Herunterladen von Bild {0}. ID: {1}", i, imagesIds[i]);
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        /// <summary>
        /// Erstellt einen WebClient, welcher Cookies speichern kann und loggt den Benutzer ein
        /// </summary>
        private static CookieAwareWebClient createWebClientAndLogin(Settings settings)
        {
            var loginData = new NameValueCollection();
            loginData.Add("j_username", settings.User);
            loginData.Add("j_password", settings.Passwort);

            var client = new CookieAwareWebClient();
            client.Headers["Accept"] = "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8";
            client.Headers["User-Agent"] = "Mozilla/5.0 (Windows NT 6.3; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/38.0.2125.111 Safari/537.36";
            client.BaseAddress = settings.URLBase;
            client.Headers["Accept-Encoding"] = "gzip,deflate";
            client.DownloadData(settings.URLTest);
            client.UploadValues(settings.LoginPage, "POST", loginData);
            return client;
        }

        /// <summary>
        /// Lädt Benutzername, Passwort, Downloadpfad und Login-URL aus der *.exe.config Datei
        /// </summary>
        private static void LoadSettingsFromConfig(Settings settings)
        {
            settings.User = ConfigurationSettings.AppSettings["user"];
            settings.Passwort = ConfigurationSettings.AppSettings["password"];
            settings.LoginPage = ConfigurationSettings.AppSettings["url-login"];
            settings.DownloadTemplate = ConfigurationSettings.AppSettings["url-sw"];
            settings.URLBase = ConfigurationSettings.AppSettings["url-base"];
            settings.URLTest = ConfigurationSettings.AppSettings["url-test"];
        }

        /// <summary>
        /// Parst die Kommandozeilenargumente 
        /// </summary>
        private static void LoadParameterFromConsole(Settings settings, String[] args)
        {
            for (int i = 0; i < args.Length; i++)
            {
                if (args[i] == "-d")
                    settings.InputFile = args[i + 1];
                if (args[i] == "-f")
                    settings.OutputPath = args[i + 1];
            }
            settings.InputFile = settings.InputFile.Replace("\\", "\\\\");
            settings.OutputPath = settings.OutputPath.Replace("\\", "\\\\");
            settings.OutputPath = settings.OutputPath.Substring(0, settings.OutputPath.Length - 1);
        }

        private static void createOutputDirectoryIfNotPresent(string path)
        {
            if (!Directory.Exists(path))
            {
                Directory.CreateDirectory(path);
            }
        }
    }


}
