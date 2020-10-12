using grapher.Layouts;
using grapher.Models.Options;
using grapher.Models.Serialized;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

namespace grapher
{
    public class AccelTypeOptions : OptionBase
    {
        #region Fields

        public static readonly Dictionary<string, LayoutBase> AccelerationTypes = new List<LayoutBase>
        {
            new OffLayout(),
            new TanhLayout()
        }.ToDictionary(k => k.Name);

        #endregion Fields

        #region Constructors

        public AccelTypeOptions(
            ComboBox accelDropdown,
            Option motivity,
            Option syncSpeed,
            Option gamma,
            Button writeButton,
            ActiveValueLabel accelTypeActiveValue)
        {
            AccelDropdown = accelDropdown;
            AccelDropdown.Items.Clear();
            AccelDropdown.Items.AddRange(AccelerationTypes.Keys.ToArray());
            AccelDropdown.SelectedIndexChanged += new System.EventHandler(OnIndexChanged);

            Motivity = motivity;
            SynchronousSpeed = syncSpeed;
            Gamma = gamma;
            WriteButton = writeButton;
            AccelTypeActiveValue = accelTypeActiveValue;

            AccelTypeActiveValue.Left = AccelDropdown.Left + AccelDropdown.Width;
            AccelTypeActiveValue.Height = AccelDropdown.Height;

            Layout("Off");
            ShowingDefault = true;
        }

        #endregion Constructors

        #region Properties
        public AccelCharts AccelCharts { get; }

        public Button WriteButton { get; }

        public ComboBox AccelDropdown { get; }

        public int AccelerationIndex
        {
            get
            {
                return AccelerationType.Index;
            }
        }

        public LayoutBase AccelerationType { get; private set; }

        public ActiveValueLabel AccelTypeActiveValue { get; }

        public Option Motivity { get; }

        public Option SynchronousSpeed { get; }

        public Option Gamma { get; }

        public override int Top 
        {
            get
            {
                return AccelDropdown.Top;
            } 
            set
            {
                AccelDropdown.Top = value;
                AccelTypeActiveValue.Top = value;
                Layout(value + AccelDropdown.Height + Constants.OptionVerticalSeperation);
            }
        }

        public override int Height
        {
            get
            {
                return AccelDropdown.Height;
            } 
        }

        public override int Left
        {
            get
            {
                return AccelDropdown.Left;
            } 
            set
            {
                AccelDropdown.Left = value;
            }
        }

        public override int Width
        {
            get
            {
                return AccelDropdown.Width;
            }
            set
            {
                AccelDropdown.Width = value;
            }
        }

        public override bool Visible
        {
            get
            {
                return AccelDropdown.Visible;
            }
        }

        private bool ShowingDefault { get; set; }

        #endregion Properties

        #region Methods

        public override void Hide()
        {
            AccelDropdown.Hide();
            AccelTypeActiveValue.Hide();
            Motivity.Hide();
            SynchronousSpeed.Hide();
            Gamma.Hide();
        }

        public void Show()
        {
            AccelDropdown.Show();
            AccelTypeActiveValue.Show();
            Layout();
        }

        public override void Show(string name)
        {
            Show();
        }

        public void SetActiveValues(int index, AccelArgs args, bool on)
        {
            if (on)
            {
                AccelerationType = AccelerationTypes.Where(t => t.Value.Index == index).FirstOrDefault().Value;
            }
            else
            {
                AccelerationType = AccelerationTypes.FirstOrDefault().Value;
            }

            AccelTypeActiveValue.SetValue(AccelerationType.Name);
            AccelDropdown.SelectedIndex = AccelerationType.Index + 1;

            Motivity.SetActiveValue(args.motivity);
            SynchronousSpeed.SetActiveValue(args.synchronousSpeed);
            Gamma.SetActiveValue(args.gamma);
        }

        public void ShowFull()
        {
            if (ShowingDefault)
            {
                AccelDropdown.Text = Constants.AccelDropDownDefaultFullText;
            }

            Left = Motivity.Left + Constants.DropDownLeftSeparation;
            Width = Motivity.Width - Constants.DropDownLeftSeparation;
        }

        public void ShowShortened()
        {
            if (ShowingDefault)
            {
                AccelDropdown.Text = Constants.AccelDropDownDefaultShortText;
            }

            Left = Motivity.Field.Left;
            Width = Motivity.Field.Width;
        }

        public void SetArgs(ref AccelArgs args)
        {
            AccelArgs defaults = DriverInterop.DefaultSettings.args.x;
            args.motivity = Motivity.Visible ? Motivity.Field.Data : defaults.motivity;
            args.synchronousSpeed = SynchronousSpeed.Visible ? SynchronousSpeed.Field.Data : defaults.synchronousSpeed;
            args.gamma = Gamma.Visible ? Gamma.Field.Data : defaults.gamma;
        }

        public AccelArgs GenerateArgs()
        {
            AccelArgs args = new AccelArgs();
            SetArgs(ref args);
            return args;
        }

        public override void AlignActiveValues()
        {
            AccelTypeActiveValue.Align();
            Motivity.AlignActiveValues();
            SynchronousSpeed.AlignActiveValues();
            Gamma.AlignActiveValues();
        }

        private void OnIndexChanged(object sender, EventArgs e)
        {
            var accelerationTypeString = AccelDropdown.SelectedItem.ToString();
            Layout(accelerationTypeString, Beneath);
            ShowingDefault = false;
        }

        private void Layout(string type, int top = -1)
        {
            AccelerationType = AccelerationTypes[type];
            Layout(top);
        }

        private void Layout(int top = -1)
        {
            if (top < 0)
            {
                top = Motivity.Top;
            }

            AccelerationType.Layout(
                Motivity,
                SynchronousSpeed,
                Gamma,
                top);
        }

        #endregion Methods
    }
}
