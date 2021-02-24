using Newtonsoft.Json;
using System;
using System.IO;

namespace grapher.Models.Serialized
{
    [Serializable]
    [JsonObject(ItemRequired = Required.Always)]
    public class GUISettings
    {
        public const string Path = ".gui-config";

        #region Constructors

        public GUISettings() {}

        #endregion Constructors

        #region Properties


        [JsonProperty(Order = 1)]
        public int DPI { get; set; }

        [JsonProperty(Order = 2)]
        public int PollRate { get; set; }

        [JsonProperty(Order = 3)]
        public bool ShowLastMouseMove { get; set; }

        [JsonProperty(Order = 4)]
        public bool ShowVelocityAndGain { get; set; }

        [JsonProperty(Order = 5)]
        public bool ActivateOnLogin { get; set; } // todo

        public bool ActivateOnLoad { get; set; }

        public string LoadPath { get; set; }


        #endregion Properties

        #region Methods

        public override bool Equals(object obj)
        {
            var other = obj as GUISettings;

            if (other == null)
            {
                return false;
            }

            return Equals(other);
        }

        public bool Equals(GUISettings other)
        {
            return DPI == other.DPI &&
                PollRate == other.PollRate &&
                ShowLastMouseMove == other.ShowLastMouseMove &&
                ShowVelocityAndGain == other.ShowVelocityAndGain &&
                ActivateOnLoad == other.ActivateOnLoad;
        }

        public override int GetHashCode()
        {
            return DPI.GetHashCode() ^
                PollRate.GetHashCode() ^
                ShowLastMouseMove.GetHashCode() ^
                ShowVelocityAndGain.GetHashCode() ^
                ActivateOnLoad.GetHashCode();
        }

        public void Save()
        {
            File.WriteAllText(Path, JsonConvert.SerializeObject(this, Formatting.Indented));
        }

        public static GUISettings MaybeLoad()
        {
            GUISettings settings = null;

            try
            {
                settings = JsonConvert.DeserializeObject<GUISettings>(
                    File.ReadAllText(Path));
            }
            catch (Exception ex)
            {
                if (!(ex is JsonException || ex is FileNotFoundException))
                {
                    throw;
                }
            }

            return settings;
        }

        #endregion Methods
    }
}
