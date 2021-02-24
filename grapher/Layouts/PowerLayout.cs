using grapher.Models.Serialized;

namespace grapher.Layouts
{
    public class PowerLayout : LayoutBase
    {
        public PowerLayout()
            : base()
        {
            Name = "Power";
            Index = (int)AccelMode.power;
            LogarithmicCharts = false;

            ScaleLayout = new OptionLayout(true, Scale);
            ExponentLayout = new OptionLayout(true, Exponent);
        }
    }
}
