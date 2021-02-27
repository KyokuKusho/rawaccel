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
            new JumpLayout(),
            new UncappedLayout(),
            new ClassicLayout(),
            new NaturalLayout(),
            new PowerLayout(),
            new MotivityLayout(),
            new OffLayout()
        }.ToDictionary(k => k.Name);

        #endregion Fields

        #region Constructors

        public AccelTypeOptions(
            ComboBox accelDropdown,
            Option acceleration,
            Option scale,
            CapOptions cap,
            Option capX,
            OffsetOptions offset,
            Option limit,
            Option exponent,
            Option midpoint,
            Button writeButton,
            ActiveValueLabel accelTypeActiveValue)
        {
            AccelDropdown = accelDropdown;
            AccelDropdown.Items.Clear();
            AccelDropdown.Items.AddRange(AccelerationTypes.Keys.ToArray());
            AccelDropdown.SelectedIndexChanged += new System.EventHandler(OnIndexChanged);

            Acceleration = acceleration;
            Scale = scale;
            Cap = cap;
            CapX = capX;
            Offset = offset;
            Limit = limit;
            Exponent = exponent;
            Midpoint = midpoint;
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

        public Option Acceleration { get; }

        public Option Scale { get; }

        public CapOptions Cap { get; }

        public Option CapX { get; }

        public OffsetOptions Offset { get; }

        public Option Limit { get; }

        public Option Exponent { get; }

        public Option Midpoint { get; }

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

            Acceleration.Hide();
            Scale.Hide();
            Cap.Hide();
            CapX.Hide();
            Offset.Hide();
            Limit.Hide();
            Exponent.Hide();
            Midpoint.Hide();
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

        public void SetActiveValues(AccelArgs args)
        {
            AccelerationType = AccelerationTypes.Where(t => t.Value.Index == (int)args.mode).FirstOrDefault().Value;
            AccelTypeActiveValue.SetValue(AccelerationType.Name);
            AccelDropdown.SelectedIndex = AccelerationType.Index;

            switch (args.mode)
            {
                case AccelMode.uncapped: Acceleration.SetActiveValue(args.accelUncapped); break;
                case AccelMode.natural: Acceleration.SetActiveValue(args.accelNatural); break;
                case AccelMode.motivity: Acceleration.SetActiveValue(args.accelMotivity); break;
                default: break;
            }

            switch (args.mode)
            {
                case AccelMode.uncapped: // fallthrough
                case AccelMode.classic: Exponent.SetActiveValue(args.power); break;
                case AccelMode.power: Exponent.SetActiveValue(args.exponent); break;
                default: break;
            }

            switch (args.mode)
            {
                case AccelMode.natural: Limit.SetActiveValue(args.limit); break;
                case AccelMode.motivity: Limit.SetActiveValue(args.motivity); break;
            }

            CapX.SetActiveValue(args.cap.x);
            Cap.SetActiveValues(args.cap.y, args.cap.y, true);
            Offset.SetActiveValue(args.offset, !args.gain);
            Scale.SetActiveValue(args.scale);
            Midpoint.SetActiveValue(args.midpoint);
        }

        public void ShowFull()
        {
            if (ShowingDefault)
            {
                AccelDropdown.Text = Constants.AccelDropDownDefaultFullText;
            }

            Left = Acceleration.Left + Constants.DropDownLeftSeparation;
            Width = Acceleration.Width - Constants.DropDownLeftSeparation;
        }

        public void ShowShortened()
        {
            if (ShowingDefault)
            {
                AccelDropdown.Text = Constants.AccelDropDownDefaultShortText;
            }

            Left = Acceleration.Field.Left;
            Width = Acceleration.Field.Width;
        }

        public void SetArgs(ref AccelArgs args)
        {
            args.gain = !Offset.IsLegacy;
            args.mode = (AccelMode)AccelerationIndex;

            if (Acceleration.Visible)
            {
                double accel = Acceleration.Field.Data;
                switch (args.mode)
                {
                    case AccelMode.uncapped: args.accelUncapped = accel; break;
                    case AccelMode.natural: args.accelNatural = accel; break;
                    case AccelMode.motivity: args.accelMotivity = accel; break;
                    default: throw new NotImplementedException();
                }
            }

            if (Exponent.Visible)
            {
                double exponent = Exponent.Field.Data;
                switch (args.mode)
                {
                    case AccelMode.uncapped: // fallthrough
                    case AccelMode.classic: args.power = exponent; break;
                    case AccelMode.power: args.exponent = exponent; break;
                    default: throw new NotImplementedException();
                }
            }

            if (Limit.Visible)
            {
                double val = Limit.Field.Data;
                switch (args.mode)
                {
                    case AccelMode.natural: args.limit = val; break;
                    case AccelMode.motivity: args.motivity = val; break;
                }
            }

            if (Scale.Visible) args.scale = Scale.Field.Data;
            if (Cap.Visible) args.cap.y = Cap.SensitivityCap;
            if (CapX.Visible) args.cap.x = CapX.Field.Data;
            if (Limit.Visible) args.limit = Limit.Field.Data;
            if (Exponent.Visible) args.exponent = Exponent.Field.Data;
            if (Offset.Visible) args.offset = Offset.Offset;
            if (Midpoint.Visible) args.midpoint = Midpoint.Field.Data;
        }

        public override void AlignActiveValues()
        {
            AccelTypeActiveValue.Align();
            Acceleration.AlignActiveValues();
            Scale.AlignActiveValues();
            Cap.AlignActiveValues();
            Offset.AlignActiveValues();
            CapX.AlignActiveValues();
            Limit.AlignActiveValues();
            Exponent.AlignActiveValues();
            Midpoint.AlignActiveValues();
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
                top = Acceleration.Top;
            }

            AccelerationType.Layout(
                Acceleration,
                Scale,
                Cap,
                CapX,
                Offset,
                Limit,
                Exponent,
                Midpoint,
                top);
        }

        #endregion Methods
    }
}
