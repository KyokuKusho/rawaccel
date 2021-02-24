using Newtonsoft.Json;
using System;
using System.Windows.Forms;
using System.Threading;
using System.Text;
using System.Drawing;
using grapher.Models.Devices;
using System.IO;
namespace grapher.Models.Serialized
{
    public class SettingsManager
    {
        #region Constructors

        public SettingsManager(
            ManagedAccel activeAccel,
            Field dpiField,
            Field pollRateField,
            ToolStripMenuItem autoWrite,
            ToolStripMenuItem showLastMouseMove,
            ToolStripMenuItem showVelocityAndGain,
            DeviceIDManager deviceIDManager)
        {
            DpiField = dpiField;
            PollRateField = pollRateField;
            AutoWriteMenuItem = autoWrite;
            ShowLastMouseMoveMenuItem = showLastMouseMove;
            ShowVelocityAndGainMoveMenuItem = showVelocityAndGain;
            DeviceIDManager = deviceIDManager;

            SetActiveMembers(activeAccel);

            GuiSettings = GUISettings.MaybeLoad();

            if (GuiSettings is null)
            {
                GuiSettings = MakeGUISettingsFromFields();
                GuiSettings.Save();
            }
            else
            {
                UpdateFieldsFromGUISettings();
            }

            UserSettings = InitUserSettings();
        }

        #endregion Constructors

        #region Properties

        public GUISettings GuiSettings { get; private set; }

        public string ActivePath { get; private set; }

        public ManagedAccel ActiveAccel { get; private set; }

        public DriverSettings ActiveSettings { get; private set; }

        public DriverSettings UserSettings { get; private set; }

        public Field DpiField { get; private set; }

        public Field PollRateField { get; private set; }

        public DeviceIDManager DeviceIDManager { get; }

        private ToolStripMenuItem AutoWriteMenuItem { get; set; }

        private ToolStripMenuItem ShowLastMouseMoveMenuItem { get; set; }

        private ToolStripMenuItem ShowVelocityAndGainMoveMenuItem { get; set; }

        public bool UserSettingsInactive
        {
            get => UserSettings != ActiveSettings;
        }

        #endregion Properties

        #region Methods

        public void DisableDriver()
        {
            var off = DriverSettings.Default;
            ActiveSettings = off;
            ActiveAccel.Settings = off;
            ActivePath = String.Empty;
            new Thread(() => ActiveAccel.Activate()).Start();
        }

        public void SetActiveMembers(ManagedAccel accel = null, String path = null)
        {
            if (accel is null)
            {
                accel = ManagedAccel.GetActive();
            }

            if (path is null)
            {
                path = String.Empty;
            }

            ActiveAccel = accel;
            ActiveSettings = accel.Settings;
            ActivePath = path;
        }

        public void UpdateFieldsFromGUISettings()
        {
            DpiField.SetToEntered(GuiSettings.DPI);
            PollRateField.SetToEntered(GuiSettings.PollRate);
            ShowLastMouseMoveMenuItem.Checked = GuiSettings.ShowLastMouseMove;
            ShowVelocityAndGainMoveMenuItem.Checked = GuiSettings.ShowVelocityAndGain;
            AutoWriteMenuItem.Checked = GuiSettings.ActivateOnLoad;
        }

        public SettingsErrors TryActivate(DriverSettings settings)
        {
            var errors = new SettingsErrors(settings);

            if (errors.Empty())
            {
                GuiSettings = MakeGUISettingsFromFields();
                GuiSettings.Save();

                UserSettings = settings;
                UserSettings.ToFile(DriverSettings.DefaultPath);

                ActiveSettings = settings;
                ActivePath = GuiSettings.LoadPath;
                
                ActiveAccel.Settings = settings;

                new Thread(() => ActiveAccel.Activate()).Start();
            }

            return errors;
        }

        public void SetHiddenOptions(DriverSettings settings)
        {
            settings.snap = UserSettings.snap;
            settings.speedCap = UserSettings.speedCap;
            settings.minimumTime = UserSettings.minimumTime;
            settings.maximumTime = UserSettings.maximumTime;
            settings.directionalMultipliers = UserSettings.directionalMultipliers;
        }

        public GUISettings MakeGUISettingsFromFields()
        {
            return new GUISettings
            {
                DPI = (int)DpiField.Data,
                PollRate = (int)PollRateField.Data,
                ShowLastMouseMove = ShowLastMouseMoveMenuItem.Checked,
                ShowVelocityAndGain = ShowVelocityAndGainMoveMenuItem.Checked,
                ActivateOnLoad = AutoWriteMenuItem.Checked,
                ActivateOnLogin = false,
                LoadPath = DriverSettings.DefaultPath
            };
        }

        private DriverSettings InitUserSettings()
        {
            if (File.Exists(GuiSettings.LoadPath))
            {
                try
                {
                    var settings = DriverSettings.FromFile(GuiSettings.LoadPath);

                    if (!GuiSettings.ActivateOnLoad || 
                        ActiveSettings.args.x.lutArgs.mode != TableMode.off ||
                        TryActivate(settings).Empty())
                    {
                        return settings;
                    }
                    
                }
                catch (JsonException e)
                {
                    System.Diagnostics.Debug.WriteLine($"bad settings: {e}");
                }
            }

            if (ActiveSettings.args.x.lutArgs.mode == TableMode.off)
            {
                ActiveSettings.ToFile(GuiSettings.LoadPath);
                return ActiveSettings;
            }
            else
            {
                var defaultSettings = DriverSettings.Default;
                defaultSettings.ToFile(GuiSettings.LoadPath);
                return defaultSettings;
            }
        }

        #endregion Methods
    }
}
