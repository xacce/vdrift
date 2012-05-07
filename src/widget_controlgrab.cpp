#include "widget_controlgrab.h"
#include "carcontrolmap_local.h"
#include "config.h"

#include <cassert>

std::string WIDGET_CONTROLGRAB::Str[] =
{
	"Add a new input", "Edit", "press", "release", "once", "held", "key", "joy", "mouse", "button", "axis", "motion"
};

WIDGET_CONTROLGRAB::CONTROLWIDGET::CONTROLWIDGET() :
	once(true),
	down(false),
	joy_index(0),
	joy_button(0),
	joy_axis(0),
	mouse_button(0),
	deadzone(0),
	exponent(1),
	gain(1)
{
	// ctor
}

WIDGET_CONTROLGRAB::WIDGET_CONTROLGRAB() :
	scale_x(0.05),
	scale_y(0.05),
	x(0.5),
	y(0.5),
	z(0),
	w(0.5),
	h(0.5),
	analog(false),
	only_one(false)
{
	// ctor
}

WIDGET_CONTROLGRAB::~WIDGET_CONTROLGRAB()
{
	//std::clog << "ControlGrab Destructor: " << setting << ", " << description << std::endl;
}

void WIDGET_CONTROLGRAB::SetAlpha(SCENENODE & scene, float newalpha)
{
	SCENENODE & topnoderef = scene.GetNode(topnode);
	addbutton.SetAlpha(topnoderef, newalpha);

	SCENENODE & ctrlnoderef = topnoderef.GetNode(ctrlnode);
	for (std::list <CONTROLWIDGET>::iterator i = controlbuttons.begin(); i != controlbuttons.end(); ++i)
	{
		i->widget.SetAlpha(ctrlnoderef, newalpha);
	}
}

void WIDGET_CONTROLGRAB::SetVisible(SCENENODE & scene, bool newvis)
{
	SCENENODE & topnoderef = scene.GetNode(topnode);
	addbutton.SetVisible(topnoderef, newvis);

	SCENENODE & ctrlnoderef = topnoderef.GetNode(ctrlnode);
	for (std::list <CONTROLWIDGET>::iterator i = controlbuttons.begin(); i != controlbuttons.end(); ++i)
	{
		i->widget.SetVisible(ctrlnoderef, newvis);
	}
}

std::string WIDGET_CONTROLGRAB::GetAction() const
{
	return active_action;
}

std::string WIDGET_CONTROLGRAB::GetDescription() const
{
	if (!tempdescription.empty())
	{
		return tempdescription;
	}
	return description;
}

void WIDGET_CONTROLGRAB::SetDescription(const std::string & newdesc)
{
	description = newdesc;
}

bool WIDGET_CONTROLGRAB::ProcessInput(
	SCENENODE & scene,
	float cursorx, float cursory,
	bool cursordown, bool cursorjustup)
{
	active_action.clear();
	tempdescription.clear();

	bool infocus = (cursorx > x - w * 0.5 &&
		cursorx < x + w * 0.5 &&
		cursory > y - h * 0.5 &&
		cursory < y + h * 0.5);

	// generate the add input tooltip, check to see if we pressed the add input button, generate an action
	SCENENODE & topnoderef = scene.GetNode(topnode);
	if (addbutton.ProcessInput(
		topnoderef, cursorx, cursory,
		cursordown, cursorjustup))
	{
		tempdescription = Str[ADDNEW_STR];
		if (cursorjustup)
		{
			active_action = "controlgrabadd:" +
				std::string(analog ? "y" : "n") + ":" +
				std::string(only_one ? "y" : "n") + ":" +
				setting;
		}
		return infocus;
	}

	// generate the input tooltip, check to see if we clicked, generate an action
	SCENENODE & ctrlnoderef = topnoderef.GetNode(ctrlnode);
	for (std::list <CONTROLWIDGET>::iterator i = controlbuttons.begin(); i != controlbuttons.end(); ++i)
	{
		if (!i->widget.ProcessInput(
			ctrlnoderef, cursorx, cursory,
			cursordown, cursorjustup))
		{
			continue;
		}

		if (i->type == "key")
		{
			std::stringstream desc;
			desc << Str[EDIT_STR] << " " << Str[KEY_STR];
			if (i->key.empty())
			{
				desc << " #" << i->keycode;
			}
			else
			{
				desc << " " << i->key;
			}
			desc << " " << (i->down ? Str[PRESS_STR] : Str[RELEASE_STR]) <<
					" (" << (i->once ? Str[ONCE_STR] : Str[HELD_STR]) << ")";
			tempdescription = desc.str();
		}
		else if (i->type == "joy")
		{
			std::stringstream desc;
			desc << Str[EDIT_STR] << " " << Str[JOY_STR] << " " << i->joy_index << " ";
			if (i->joy_type == "button")
			{
				desc << Str[BUTTON_STR] << " " << i->joy_button <<
					" " << (i->down ? Str[PRESS_STR] : Str[RELEASE_STR]) <<
					" (" << (i->once ? Str[ONCE_STR] : Str[HELD_STR]) << ")";
			}
			else if (i->joy_type == "axis")
			{
				desc << Str[AXIS_STR] << " " << i->joy_axis << " " <<
					"(" << (i->joy_axis_type == "negative" ? "-" : "+") << ")";
			}
			tempdescription = desc.str();
		}
		else if (i->type == "mouse")
		{
			std::stringstream desc;
			desc << Str[EDIT_STR] + " " << Str[MOUSE_STR] << " ";
			if (i->mouse_type == "button")
			{
				desc << Str[BUTTON_STR] << " " << i->mouse_button <<
					" " << (i->down ? Str[PRESS_STR] : Str[RELEASE_STR]) <<
					" (" << (i->once ? Str[ONCE_STR] : Str[HELD_STR]) << ")";
			}
			else if (i->mouse_type == "motion")
			{
				desc << Str[MOTION_STR] << " " << i->mouse_motion;
			}
			tempdescription = desc.str();
		}

		// action string is based on the DebugPrint string representation of a CONTROL
		if (cursorjustup)
		{
			CARCONTROLMAP_LOCAL::CONTROL newctrl;
			newctrl.deadzone = i->deadzone;
			newctrl.exponent = i->exponent;
			newctrl.gain = i->gain;
			newctrl.onetime = i->once;
			newctrl.type = CARCONTROLMAP_LOCAL::CONTROL::UNKNOWN;

			if (i->type == "key")
			{
				newctrl.type = CARCONTROLMAP_LOCAL::CONTROL::KEY;
				std::stringstream keycodestr(i->keycode);
				keycodestr >> newctrl.keycode;
				newctrl.keypushdown = i->down;
			}
			else if (i->type == "joy")
			{
				newctrl.type = CARCONTROLMAP_LOCAL::CONTROL::JOY;
				newctrl.joynum = i->joy_index;
				newctrl.joytype = CARCONTROLMAP_LOCAL::CONTROL::JOYBUTTON;
				newctrl.joybutton = i->joy_button;
				newctrl.joypushdown = i->down;

				if (i->joy_type == "axis")
				{
					newctrl.joytype = CARCONTROLMAP_LOCAL::CONTROL::JOYAXIS;
					newctrl.joyaxistype = CARCONTROLMAP_LOCAL::CONTROL::POSITIVE;
					newctrl.joyaxis = i->joy_axis;

					if (i->joy_axis_type == "negative")
					{
						newctrl.joyaxistype = CARCONTROLMAP_LOCAL::CONTROL::NEGATIVE;
					}
					else if (i->joy_axis_type == "both")
					{
						newctrl.joyaxistype = CARCONTROLMAP_LOCAL::CONTROL::BOTH;
					}
				}
				else if (i->joy_type == "hat")
				{
					newctrl.joytype = CARCONTROLMAP_LOCAL::CONTROL::JOYHAT;
				}
			}
			else if (i->type == "mouse")
			{
				newctrl.type = CARCONTROLMAP_LOCAL::CONTROL::MOUSE;
				newctrl.mousetype = CARCONTROLMAP_LOCAL::CONTROL::MOUSEBUTTON;
				newctrl.mbutton = i->mouse_button;
				newctrl.mouse_push_down = i->down;

				if (i->mouse_type == "motion")
				{
					newctrl.mousetype = CARCONTROLMAP_LOCAL::CONTROL::MOUSEMOTION;
					newctrl.mdir = CARCONTROLMAP_LOCAL::CONTROL::RIGHT;

					if (i->mouse_motion == "up")
					{
						newctrl.mdir = CARCONTROLMAP_LOCAL::CONTROL::UP;
					}
					else if (i->mouse_motion == "down")
					{
						newctrl.mdir = CARCONTROLMAP_LOCAL::CONTROL::DOWN;
					}
					else if (i->mouse_motion == "left")
					{
						newctrl.mdir = CARCONTROLMAP_LOCAL::CONTROL::LEFT;
					}
				}
			}

			std::stringstream controlstring;
			newctrl.DebugPrint(controlstring);
			active_action = "controlgrabedit:" + controlstring.str() + setting;
		}

		break;
	}

	return infocus;
}

void WIDGET_CONTROLGRAB::SetupDrawable(
	SCENENODE & scene,
	const std::vector <std::tr1::shared_ptr<TEXTURE> > & texturevector,
	const std::string & newsetting,
	const CONFIG & c,
	const FONT & font,
	float scalex, float scaley,
	float centerx, float centery, float newz,
	bool newanalog, bool newonly_one)
{
	assert(!newsetting.empty());
	assert(texturevector.size() == END);
	for (int i = 0; i < END; ++i) assert(texturevector[i]);

	setting = newsetting;
	textures = texturevector;

	topnode = scene.AddNode();
	SCENENODE & topnoderef = scene.GetNode(topnode);
	ctrlnode = topnoderef.AddNode();

	x = centerx;
	y = centery;
	z = newz;
	h = scaley;
	w = 0.5;

	scale_x = scalex;
	scale_y = scaley;

	analog = newanalog;
	only_one = newonly_one;

	float bw = scalex * 0.8 * (4.0 / 3.0);
	float bh = scaley * 0.8;

	addbutton.SetupDrawable(
		topnoderef,
		textures[ADD], textures[ADDSEL], textures[ADDSEL],
		font, "", bw, bh,
		x, y, 0, 0, z,
		m_r ,m_g, m_b);

	LoadControls(scene, c, font);
}

void WIDGET_CONTROLGRAB::LoadControls(SCENENODE & scene, const CONFIG & c, const FONT & font)
{
	assert(textures.size() == END);
	assert(!setting.empty()); //ensure that we've already done a SetupDrawable

	SCENENODE & parentnode = scene.GetNode(topnode).GetNode(ctrlnode);
	parentnode.Clear();
	controlbuttons.clear();

	for (CONFIG::const_iterator section = c.begin(); section != c.end(); ++section)
	{
		std::string controlname;
		c.GetParam(section, "name",  controlname);

		if (controlname == setting)
		{
			controlbuttons.push_back(CONTROLWIDGET());
			CONTROLWIDGET & button = controlbuttons.back();
			c.GetParam(section, "type", button.type);
			c.GetParam(section, "once", button.once);
			c.GetParam(section, "down", button.down);
			c.GetParam(section, "key", button.key);
			c.GetParam(section, "keycode", button.keycode);
			c.GetParam(section, "joy_type", button.joy_type);
			c.GetParam(section, "joy_index", button.joy_index);
			c.GetParam(section, "joy_button", button.joy_button);
			c.GetParam(section, "joy_axis", button.joy_axis);
			c.GetParam(section, "joy_axis_type", button.joy_axis_type);
			c.GetParam(section, "mouse_type", button.mouse_type);
			c.GetParam(section, "mouse_motion", button.mouse_motion);
			c.GetParam(section, "mouse_button", button.mouse_button);
			c.GetParam(section, "deadzone", button.deadzone);
			c.GetParam(section, "exponent", button.exponent);
			c.GetParam(section, "gain", button.gain);

			std::tr1::shared_ptr<TEXTURE> tex_unsel;
			std::tr1::shared_ptr<TEXTURE> tex_sel;
			if (button.type == "key")
			{
				tex_unsel = textures[KEY];
				tex_sel = textures[KEYSEL];
			}
			else if (button.type == "joy")
			{
				if (button.joy_type == "button")
				{
					tex_unsel = textures[JOYBTN];
					tex_sel = textures[JOYBTNSEL];
				}
				else if (button.joy_type == "axis")
				{
					tex_unsel = textures[JOYAXIS];
					tex_sel = textures[JOYAXISSEL];
				}
			}
			else if (button.type == "mouse")
			{
				tex_unsel = textures[MOUSE];
				tex_sel = textures[MOUSESEL];
			}

			float bw = scale_x * 0.8 * (4.0 / 3.0);
			float bh = scale_y * 0.8;
			float bx = x + bw * controlbuttons.size();
			float by = y;

			button.widget.SetupDrawable(
				parentnode,
				tex_unsel, tex_sel, tex_sel,
				font, "", bw, bh,
				bx, by, 0, 0, z,
				m_r, m_g, m_b);
		}
	}
}
