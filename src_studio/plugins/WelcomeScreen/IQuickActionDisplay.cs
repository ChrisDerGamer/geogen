namespace GeoGen.Studio.PlugIns
{
    using GeoGen.Studio.PlugInLoader;

    /// <summary>
    /// Displays a list of quick actions provided by other plug-ins.
    /// </summary>
    public interface IQuickActionDisplay : IPlugInInterface
    {
        /// <summary>
        /// Registers a quick action to be displayed in the component.
        /// </summary>
        /// <param name="quickAction">The quick action.</param>
        void RegisterQuickAction(QuickAction quickAction);
    }
}