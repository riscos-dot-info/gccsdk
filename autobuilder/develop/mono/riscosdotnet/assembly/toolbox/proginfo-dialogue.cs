//
// toolbox/proginfo-dialogue.cs - a binding of the RISC OS API to C#.
//
// Author: Lee Noar (leenoar@sky.com)
//
 
using System;
using System.Runtime.InteropServices;

namespace riscos
{
	public static partial class Toolbox
	{
		/*! \brief Encapsulates a Toolbox %ProgInfo Dialogue Box.  */
		public class ProgInfo : Object
		{
			static class Method
			{
				public const int GetWindowID = 0;
				public const int SetVersion = 1;
				public const int GetVersion = 2;
				public const int SetLicenceType = 3;
				public const int GetLicenceType = 4;
				public const int SetTitle = 5;
				public const int GetTitle = 6;
			}

			/*! \brief The possible licence types that the Licence property may be assigned.  */
			public enum LicenceType
			{
				PublicDomain,
				SingleUser,
				SingleMachine,
				Site,
				Network,
				Authority
			}

			/*! \brief Default Toolbox events that can be generated by the %Toolbox %ProgInfo object.  */
			public static class EventCode
			{
				public const int AboutToBeShown = 0x82b40;
				public const int DialogueCompleted = 0x82b41;
			}

			/*! \brief The signature of a AboutToBeShown event handler.  */
			public delegate void AboutToBeShownEventHandler (object sender, AboutToBeShownEventArgs e);

			/*! \brief The event handlers that will be called just before this %ProgInfo is shown.
			 *
			 * Handlers should have the signature:
			 * \code
			 * void handler_name (object sender, AboutToBeShownEventArgs e);
			 * \endcode
			 * and can be added to the list with:
			 * \code
			 * ProgInfoObject.AboutToBeShown += handler_name;
			 * \endcode  */
			public event AboutToBeShownEventHandler AboutToBeShown;

			/*! \brief The signature of a DialogueComplete event handler.  */
			public delegate void DialogueCompleteEventHandler (object sender, ToolboxEventArgs e);

			/*! \brief The event handlers that will be called when this dialogue is hidden.
			 *
			 * Handlers should have the signature:
			 * \code
			 * void handler_name (object sender, ToolboxEventArgs e);
			 * \endcode
			 * and can be added to the list with:
			 * \code
			 * ProgInfoObject.DialogueComplete += handler_name;
			 * \endcode  */
			public event ToolboxEventHandler DialogueComplete;

			/*! \brief Create a %Toolbox %ProgInfo Dialogue from the named template in the
			 * Resource file.
			 * \param[in] resName The name of the %ProgInfo template to use.  */
			public ProgInfo (string resName) : base (resName)
			{
			}

			/*! \brief Create a %Toolbox %ProgInfo from the template data given.
			 * \param[in] templateData Pointer to the window template.  */
			public ProgInfo (IntPtr templateData)
			{
				Create (templateData);
			}

			/*! \brief Create a %Toolbox %ProgInfo from an object that already exists.
			 * \param [in] objectID The Toolbox ID of the existing object.  */
			public ProgInfo (uint objectID) : base (objectID)
			{
			}

			/*! \brief Get the ID of the underlying Window object.  */
			public uint WindowID
			{
				get { return CallMethod_GetR0 (Method.GetWindowID); }
			}

			/*! \brief Gets or sets the version string used in the %ProgInfo Dialogue's Window.  */
			public string Version
			{
				set { SetText (Method.SetVersion, value); }
				get { return GetText (Method.GetVersion); }
			}

			/*! \brief Gets or sets the text which is to be used in the title bar
			 * of this %ProgInfo dialogue.  */
			public string Title
			{
				set { SetText (Method.SetTitle, value); }
				get { return GetText (Method.GetTitle); }
			}

			/*! \brief Gets or sets the licence type used in this %ProgInfo Dialogue's Window.  */
			public LicenceType Licence
			{
				set { CallMethod_SetR3 (Method.SetLicenceType, (uint)value); }
				get { return (LicenceType)CallMethod_GetR0 (Method.GetLicenceType); }
			}

			/*! \brief Check if the given event is relevant to the %ProgInfo dialogue and call the
			 * associated event handlers.  */
			public override void Dispatch (ToolboxEvent ev)
			{
				switch (ev.ToolboxArgs.Header.EventCode)
				{
				case EventCode.AboutToBeShown:
					if (AboutToBeShown != null)
						AboutToBeShown (this, new AboutToBeShownEventArgs (ev.ToolboxArgs.RawEventData));
					break;
				case EventCode.DialogueCompleted:
					if (DialogueComplete != null)
						DialogueComplete (this, ev.ToolboxArgs);
					break;
				}
			}
		}
	}
}
