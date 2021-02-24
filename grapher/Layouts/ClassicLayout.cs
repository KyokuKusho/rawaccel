using grapher.Models.Serialized;

namespace grapher.Layouts
{
    public class ClassicLayout : LayoutBase
    {
        public ClassicLayout()
            : base()
        {
            Name = "Classic";
            Index = (int)AccelMode.classic;

            CapLayout = new OptionLayout(true, Cap);
            CapXLayout = new OptionLayout(true, CapX);
            OffsetLayout = new OptionLayout(true, Offset);
            ExponentLayout = new OptionLayout(true, Exponent);
        }
    }
}
