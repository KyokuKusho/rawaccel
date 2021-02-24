using grapher.Models.Serialized;

namespace grapher.Layouts
{
    public class UncappedLayout : LayoutBase
    {
        public UncappedLayout()
            : base()
        {
            Name = "Uncapped";
            Index = (int)AccelMode.uncapped;
            LogarithmicCharts = false;

            AccelLayout = new OptionLayout(true, Acceleration);
            OffsetLayout = new OptionLayout(true, Offset);
            ExponentLayout = new OptionLayout(true, Exponent);
        }
    }
}
