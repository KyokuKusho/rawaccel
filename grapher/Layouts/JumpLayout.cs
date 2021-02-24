using grapher.Models.Serialized;

namespace grapher.Layouts
{
    public class JumpLayout : LayoutBase
    {
        public JumpLayout()
            : base()
        {
            Name = "Jump";
            Index = (int)AccelMode.jump;
            LogarithmicCharts = false;

            CapLayout = new OptionLayout(true, "Y");
            CapXLayout = new OptionLayout(true, "X");
        }
    }
}
