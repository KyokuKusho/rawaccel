using grapher.Models.Options;
using System.Windows.Forms;

namespace grapher.Layouts
{
    public abstract class LayoutBase
    {
        public const string Motivity = "Motivity";
        public const string SynchronousSpeed = "Sync. Speed";
        public const string Gamma = "Gamma";

        public LayoutBase()
        {
            MotivityLayout = new OptionLayout(false, string.Empty);
            SynchronousSpeedLayout = new OptionLayout(false, string.Empty);
            GammaLayout = new OptionLayout(false, string.Empty);

            ButtonEnabled = true;
            LogarithmicCharts = false;
        }

        /// <summary>
        ///  Gets or sets mapping from acceleration type to identifying integer.
        ///  Must match accel_mode defined in rawaccel-settings.h
        /// </summary>
        public int Index { get; protected set; }

        public string Name { get; protected set; }

        public bool LogarithmicCharts { get; protected set; }

        protected bool ButtonEnabled { get; set; }

        protected OptionLayout MotivityLayout { get; set; }

        protected OptionLayout SynchronousSpeedLayout { get; set; }

        protected OptionLayout GammaLayout { get; set; }

        public void Layout(
            IOption motivityOption,
            IOption synchronousSpeedOption,
            IOption gammaOption,
            int top)
        {

            IOption previous = null;

            foreach (var option in new (OptionLayout, IOption)[] {
                (MotivityLayout, motivityOption),
                (SynchronousSpeedLayout, synchronousSpeedOption),
                (GammaLayout, gammaOption)})
            {
                option.Item1.Layout(option.Item2);

                if (option.Item2.Visible)
                {
                    if (previous != null)
                    {
                        option.Item2.SnapTo(previous);
                    }
                    else
                    {
                        option.Item2.Top = top;
                    }

                    previous = option.Item2;
                }
            }
        }

        public void Layout(
            IOption motivityOption,
            IOption synchronousSpeedOption,
            IOption gammaOption)
        {
            Layout(motivityOption,
                synchronousSpeedOption,
                gammaOption,
                motivityOption.Top);
        }
    }
}
