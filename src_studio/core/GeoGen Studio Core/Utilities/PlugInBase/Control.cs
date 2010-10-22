﻿using System.Windows;
using System.Windows.Controls;
using System.ComponentModel;
using GeoGen.Studio.Utilities.Messaging;

namespace GeoGen.Studio.Utilities.PlugInBase
{
    class Control: UserControl, INotifyPropertyChanged
    {
        public static readonly RoutedEvent MessageThrownEvent =
            EventManager.RegisterRoutedEvent("MessageThrown", RoutingStrategy.Bubble,
            typeof(MessageThrownEventHandler), typeof(Control));

        public event PropertyChangedEventHandler PropertyChanged;

        public event RoutedEventHandler MessageThrown
        {
            add
            {
                AddHandler(MessageThrownEvent, value);
            }
            remove
            {
                RemoveHandler(MessageThrownEvent, value);
            }
        }

        protected void ThrowMessage(Message message)
        {
            RaiseEvent(new MessageThrownEventArgs(message, Control.MessageThrownEvent));
        }

        protected void NotifyPropertyChanged(string info)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(info));
            }
        }
    }
}
