﻿namespace GeoGen.Studio.PlugIns.FullScreen
{
	using System;
	using System.Collections.Generic;
	using System.ComponentModel;
	using System.Windows;
	using System.Windows.Controls;
	using System.Windows.Data;
	using System.Windows.Input;
	using System.Windows.Media;	
	using GeoGen.Studio.PlugInLoader;
	using GeoGen.Studio.PlugIns.Interfaces;
	using GeoGen.Studio.PlugIns.MenuBars;
	using GeoGen.Studio.PlugIns.ToolBars;
	using GeoGen.Studio.Utilities;
	using GeoGen.Studio.Utilities.PlugInBase;

	public sealed class FullScreen: ObjectBase, IPlugIn, INotifyPropertyChanged
	{
		private Window mainWindow;
		private List<Control> bars = new List<Control>();

		private ICommand toggleFullScreenCommand;

		private WindowState windowStateBackup;

		public const string ICON_PATH_PREFIX = "pack://application:,,,/GeoGen.Studio.PlugIns.FullScreen;component/Images/Icons/";

		public bool IsFullScreen
		{
			get
			{
				return this.mainWindow.WindowStyle == WindowStyle.None;
			}
		}

		public FullScreen()
		{
			this.toggleFullScreenCommand = new RelayCommand(p => this.ToggleFullScreen());			
		}

		public void Register(IMainWindow mainWindow){
			this.mainWindow = mainWindow.Window;

			this.mainWindow.Deactivated += delegate {
				if (this.mainWindow.WindowStyle == WindowStyle.None)
				{
					this.ToggleFullScreen();
				}				
			};			
		}

		public void Register(IMainWindowToolBar toolBar)
		{
			toolBar.AddItem(
				new ToolBarCheckableButton(
					icon: FullScreen.ICON_PATH_PREFIX  + "fullscreen.png", 
					priority: -10, 
					command: this.toggleFullScreenCommand, 
					toolTip: "Toggle full screen mode",					
					isCheckedBinding: new Binding("IsFullScreen"),
					dataContext: this
				)
			);

			bars.Add(toolBar.Control);
		}

		public void Register(IMenuBar menuBar){
			menuBar.AddMenu(
				new MenuEntry(
					header: "View",
					items: new MenuEntryObservableCollection()
					{
						new MenuEntry(
							header: "Full Screen",
							priority: -10,
							command: this.toggleFullScreenCommand,
							inputGestureText: "F11",
							icon: FullScreen.ICON_PATH_PREFIX  + "fullscreen.png",
							isCheckedBinding: new Binding("IsFullScreen"),
							dataContext: this,
							isCheckable: true
						)
					}
				)
			);

			bars.Add(menuBar.Control);
		}

		public void Register(IStatusBar statusBar)
		{
			bars.Add(statusBar.Control);
		}

		private void ToggleFullScreen()
		{
			if (this.mainWindow.WindowStyle != WindowStyle.None)
			{
				this.windowStateBackup = this.mainWindow.WindowState;
				this.mainWindow.WindowState = WindowState.Normal;
				
				this.mainWindow.WindowStyle = WindowStyle.None;
				this.mainWindow.WindowState = WindowState.Maximized;
				this.mainWindow.Topmost = true;
				this.mainWindow.Top = 0;
				this.mainWindow.Left = 0;

				this.HideBars();

				this.mainWindow.MouseMove += this.HandleMouseMove;
			}
			else
			{
				this.mainWindow.ResizeMode = ResizeMode.CanResize;
				this.mainWindow.WindowState = this.windowStateBackup;
				this.mainWindow.WindowStyle = WindowStyle.SingleBorderWindow;
				this.mainWindow.Topmost = false;

				this.ShowBars();

				this.mainWindow.MouseMove -= this.HandleMouseMove;
			}

			GeoGen.Studio.App.Current.Dispatcher.BeginInvoke((Action)delegate{this.OnPropertyChanged("IsFullScreen");});
		}

		private void ShowBars(){
			foreach (Control bar in this.bars)
			{
				bar.Visibility = Visibility.Visible;
			}
		}

		private void HideBars()
		{
			foreach (Control bar in this.bars)
			{
				bar.Visibility = Visibility.Collapsed;
			}
		}

		private void HandleMouseMove(object sender, MouseEventArgs args){
			// Display the bars if the mouse is on top border of the screen
			if (args.MouseDevice.GetPosition(this.mainWindow).Y <= 1)
			{
				this.ShowBars();
				this.mainWindow.MouseMove -= this.HandleMouseMove;
				
				foreach (Control bar in this.bars){
					bar.MouseLeave += this.HandleMouseLeave;
					bar.IsHitTestVisible = true;
				}
			}
		}

		private void HandleMouseLeave(object sender, MouseEventArgs args)
		{
			// Check if the mouse is still above some of the bars
			bool barWasHit = false;
			VisualTreeHelper.HitTest(
				reference: this.mainWindow,
				filterCallback: null,
				resultCallback: (HitTestResultCallback) delegate(HitTestResult p){
					barWasHit |= this.bars.Contains(p.VisualHit as Control);
					return HitTestResultBehavior.Continue;//barWasHit ? HitTestResultBehavior.Stop : HitTestResultBehavior.Continue;
				},
				hitTestParameters: new PointHitTestParameters(args.MouseDevice.GetPosition(this.mainWindow))
			);

			//VisualTreeHelper.

			// If the mouse is away from any bar, hide the bars
			if (!barWasHit)
			{
				foreach (Control bar in this.bars)
				{
					bar.MouseLeave -= this.HandleMouseLeave;
				}

				this.HideBars();
				this.mainWindow.MouseMove += this.HandleMouseMove;
			}
			else
			{
				Console.Write("sad");
			}			
		}
	}
}