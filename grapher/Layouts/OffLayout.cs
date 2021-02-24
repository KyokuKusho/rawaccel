using grapher.Models.Serialized;

namespace grapher.Layouts
{
    public class OffLayout : LayoutBase
    {
        public OffLayout()
            : base()
        {
            Name = "Off";
            Index = (int)AccelMode.noaccel;
        }
    }
}
