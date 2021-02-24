using grapher.Models.Calculations;
using grapher.Models.Devices;
using grapher.Models.Mouse;
using grapher.Models.Options;
using grapher.Models.Serialized;
using System;
using System.Drawing;
using System.Windows.Forms;
using System.Windows.Forms.DataVisualization.Charting;

namespace grapher
{
    public class AccelGUI
    {

        #region Constructors

        public AccelGUI(
            RawAcceleration accelForm,
            AccelCalculator accelCalculator,
            AccelCharts accelCharts,
            SettingsManager settingsManager,
            ApplyOptions applyOptions,
            Button writeButton,
            ButtonBase toggleButton,
            MouseWatcher mouseWatcher,
            ToolStripMenuItem scaleMenuItem,
            DeviceIDManager deviceIDManager)
        {
            AccelForm = accelForm;
            AccelCalculator = accelCalculator;
            AccelCharts = accelCharts;
            ApplyOptions = applyOptions;
            WriteButton = writeButton;
            DisableButton = (CheckBox)toggleButton;
            ScaleMenuItem = scaleMenuItem;
            SM = settingsManager;
            DefaultButtonFont = WriteButton.Font;
            SmallButtonFont = new Font(WriteButton.Font.Name, WriteButton.Font.Size * Constants.SmallButtonSizeFactor);
            MouseWatcher = mouseWatcher;

            DeviceIDManager = deviceIDManager;
            DeviceIDManager.DeviceIDsMenuItem.DropDownItemClicked += (s, e) =>
            {
                UpdateActiveSettingsFromFields();
            };

            ScaleMenuItem.Click += OnScaleMenuItemClick;
            WriteButton.Click += OnWriteButtonClick;
            DisableButton.Click += DisableDriverEventHandler;
            AccelForm.FormClosing += SaveGUISettingsOnClose;
            AccelForm.KeyDown += AccelForm_KeyDown;

            ButtonTimerInterval = Convert.ToInt32(DriverSettings.WriteDelayMs);
            ButtonTimer = new Timer();
            ButtonTimer.Tick += OnButtonTimerTick;

            ChartRefresh = SetupChartTimer();

            RefreshUser();
            RefreshActive();
            SetupButtons();

            // TODO: The below removes an overlapping form from the anisotropy panel.
            // Figure out why and remove the overlap and below.
            ApplyOptions.Directionality.Show();
            ApplyOptions.Directionality.Hide();
        }

        #endregion Constructors

        #region Properties

        public RawAcceleration AccelForm { get; }

        public AccelCalculator AccelCalculator { get; }

        public AccelCharts AccelCharts { get; }

        public SettingsManager SM { get; }

        public ApplyOptions ApplyOptions { get; }

        public Button WriteButton { get; }

        public CheckBox DisableButton { get; }

        public Timer ButtonTimer { get; }

        public MouseWatcher MouseWatcher { get; }

        public ToolStripMenuItem ScaleMenuItem { get; }

        public DeviceIDManager DeviceIDManager { get; }

        private Timer ChartRefresh { get; }

        private Font SmallButtonFont { get; }

        private Font DefaultButtonFont { get; }

        private int ButtonTimerInterval { get; }

        #endregion Properties

        #region Methods

        private void SaveGUISettingsOnClose(Object sender, FormClosingEventArgs e)
        {
            var guiSettings = SM.MakeGUISettingsFromFields();

            if (!SM.GuiSettings.Equals(guiSettings))
            {
                guiSettings.Save();
            }
        }

        public void UpdateActiveSettingsFromFields()
        {
            var settings = ApplyOptions.MakeSettings();
            settings.deviceID = DeviceIDManager.ID;
            SM.SetHiddenOptions(settings);

            ButtonDelay(WriteButton);

            SettingsErrors errors = SM.TryActivate(settings);
            if (errors.Empty())
            {
                RefreshActive();
            }
            else
            {
                new MessageDialog(errors.ToString(), "bad input").ShowDialog();
            }
        }

        public void UpdateInputManagers()
        {
            MouseWatcher.UpdateHandles(SM.ActiveSettings.deviceID);
            DeviceIDManager.Update(SM.ActiveSettings.deviceID);
        }

        public void RefreshActive()
        {
            UpdateGraph();
            UpdateInputManagers();
        }

        public void RefreshUser()
        {
            UpdateShownActiveValues(SM.UserSettings);
        }

        public void UpdateGraph()
        {
            AccelCharts.Calculate(
                SM.ActiveAccel,
                SM.ActiveSettings);
            AccelCharts.Bind();
        }

        public void UpdateShownActiveValues(DriverSettings args)
        {
            AccelForm.ResetAutoScroll();
            AccelCharts.ShowActive(args);
            ApplyOptions.SetActiveValues(args);
        }

        private Timer SetupChartTimer()
        {
            Timer chartTimer = new Timer();
            chartTimer.Enabled = true;
            chartTimer.Interval = 10;
            chartTimer.Tick += OnChartTimerTick;
            return chartTimer;
        }

        private void SetupButtons()
        {
            WriteButton.Top = Constants.SensitivityChartAloneHeight - Constants.ButtonVerticalOffset;
            
            DisableButton.Appearance = Appearance.Button;
            DisableButton.FlatStyle = FlatStyle.System;
            DisableButton.TextAlign = ContentAlignment.MiddleCenter;
            DisableButton.Size = WriteButton.Size;
            DisableButton.Top = WriteButton.Top;

            SetButtonDefaults();
        }

        private void SetButtonDefaults()
        {
            DisableButton.Font = DefaultButtonFont;
            DisableButton.Text = "Disable";
            DisableButton.Enabled = true;
            DisableButton.Update();

            WriteButton.Font = DefaultButtonFont;
            WriteButton.Text = Constants.WriteButtonDefaultText;
            WriteButton.Enabled = true;
            WriteButton.Update();
        }

        private void OnScaleMenuItemClick(object sender, EventArgs e)
        {
            UpdateGraph();
        }

        private void OnWriteButtonClick(object sender, EventArgs e)
        {
            UpdateActiveSettingsFromFields();
        }

        private void DisableDriverEventHandler(object sender, EventArgs e)
        {
            ButtonDelay(DisableButton);
            SM.DisableDriver();
            RefreshActive();
        }

        private void OnButtonTimerTick(object sender, EventArgs e)
        {
            ButtonTimer.Stop();
            DeviceIDManager.DeviceIDsMenuItem.Enabled = true;
            SetButtonDefaults();
            AccelForm.KeyDown += AccelForm_KeyDown;
        }

        private void StartButtonTimer()
        {
            ButtonTimer.Interval = ButtonTimerInterval;
            ButtonTimer.Start();
        }

        private void ButtonDelay(ButtonBase btn)
        {
            AccelForm.KeyDown -= AccelForm_KeyDown;

            btn.Font = SmallButtonFont;
            btn.Text = $"{Constants.ButtonDelayText} : {ButtonTimerInterval} ms";
            DisableButton.Enabled = false;
            WriteButton.Enabled = false;
            DeviceIDManager.DeviceIDsMenuItem.Enabled = false;

            DisableButton.Update();
            WriteButton.Update();
            StartButtonTimer();
        }

        private void AccelForm_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Control && e.KeyCode == Keys.Q)
            {
                DisableDriverEventHandler(sender, e);
                e.Handled = true;
                e.SuppressKeyPress = true;
            }
        }

        private void OnChartTimerTick(object sender, EventArgs e)
        {
            AccelCharts.DrawLastMovement();
            MouseWatcher.UpdateLastMove();
        }

        #endregion Methods
    }

}
