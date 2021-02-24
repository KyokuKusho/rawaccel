using grapher.Models.Serialized;

namespace grapher.Layouts
{
    public class DefaultLayout : LayoutBase
    {
        public DefaultLayout()
            : base()
        {
            Name = "Default";
            Index = (int)AccelMode.noaccel;
            LogarithmicCharts = false;

            AccelLayout = new OptionLayout(true, Acceleration);
            ScaleLayout = new OptionLayout(true, Scale);
            CapLayout = new OptionLayout(true, Cap);
            CapXLayout = new OptionLayout(true, CapX);
            OffsetLayout = new OptionLayout(true, Offset);
            LimitLayout = new OptionLayout(true, Limit);
            ExponentLayout = new OptionLayout(true, Exponent);
            MidpointLayout = new OptionLayout(true, Midpoint);
        }
    }
}
