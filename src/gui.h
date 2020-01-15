#ifndef GUI_H_
#define GUI_H_

#include <vector>
#include <initializer_list>
#include <string>
#include <cmath>
#include "window.h"
#include "texture.h"

namespace GUI {
	const float InfFloat = 1e10f;
	
	float getScalingFactor();
	void setScalingFactor(float);
	
	// Externally referenced colors
	extern const float BackgroundColor[3];
	
	// TODO: using Point2D = Vec2f;
	class Point2D {
	public:
		float x = 0, y = 0;
		Point2D() = default;
		Point2D(int x_, int y_): x(float(x_)), y(float(y_)) {}
		Point2D(float x_, float y_): x(x_), y(y_) {}
		Point2D operator+(const Point2D& r) const { return Point2D(x + r.x, y + r.y); }
		Point2D operator-(const Point2D& r) const { return Point2D(x - r.x, y - r.y); }
		Point2D operator*(const Point2D& r) const { return Point2D(x * r.x, y * r.y); }
		Point2D operator/(const Point2D& r) const { return Point2D(x / r.x, y / r.y); }
		Point2D operator*(float r) const { return Point2D(x * r, y * r); }
		Point2D operator/(float r) const { return Point2D(x / r, y / r); }
		bool operator>=(const Point2D& r) const { return x >= r.x && y >= r.y; }
		bool operator<=(const Point2D& r) const { return x <= r.x && y <= r.y; }
		Point2D round() const { return Point2D(std::round(x), std::round(y)); }
	};

	class Position {
	public:
		Point2D relative, offset;
		Position() = default;
		Position(const Point2D& relative_, const Point2D& offset_): relative(relative_), offset(offset_) {}
		Position(float xrel, float yrel, float xoff, float yoff): relative(xrel, yrel), offset(xoff, yoff) {}
		Point2D compute(const Point2D& parentSize) const { return parentSize * relative + offset * getScalingFactor(); }
	};

	class Form; // Depends on Area

	class Control {
	public:
		Position upperLeft, lowerRight;
		bool focusable = true, active = true;
		
		Control() = default;
		Control(const Position& ul, const Position& lr, bool focusable_): upperLeft(ul), lowerRight(lr), focusable(focusable_) {}

		virtual void addChild(Control* c) { mChildren.push_back(c); }
		virtual void addChild(std::initializer_list<Control*> c) { for (Control* curr: c) mChildren.push_back(curr); }
		virtual std::vector<Control*>& children() { return mChildren; }
		const std::vector<Control*>& realChildren() const { return mChildren; }
		
		bool focused(const Form& form) const;
		void getFocus(Form& form) const;

		// Update subtree
		virtual void updateAll(const Point2D& parentPos, const Point2D& parentSize, Form& form) {
			Point2D ul = upperLeft.compute(parentSize) + parentPos;
			Point2D lr = lowerRight.compute(parentSize) + parentPos;
			if (active) {
				for (Control* c: mChildren) c->updateAll(ul, lr - ul, form);
				update(ul, lr, form);
			}
		}
		// Render subtree
		virtual void renderAll(const Point2D& parentPos, const Point2D& parentSize, const Form& form, unsigned int channel = 0) const {
			Point2D ul = upperLeft.compute(parentSize) + parentPos;
			Point2D lr = lowerRight.compute(parentSize) + parentPos;
			if (active) {
				render(ul, lr, form);
				for (const Control* c: mChildren) c->renderAll(ul, lr - ul, form, channel);
			}
		}

	private:
		std::vector<Control*> mChildren;

		virtual void update(const Point2D&, const Point2D&, Form&) {}
		virtual void render(const Point2D&, const Point2D&, const Form&) const {}
	};

	class Area: public Control {
	public:
		Area() = default;
		Area(const Position& ul, const Position& lr, bool focusable = false): Control(ul, lr, focusable) {}
	};
	
	class Form {
	public:
		Area* area = nullptr;
		
		Form() = default;
		Form(Area* area_): area(area_) { updateTabIndex(area); }

		const Control* focus() const { return mFocus; }
		void setFocus(const Control* control) { mFocus = control; }

		bool mouseIgnored() { return mMouseIgnored; }
		void awareMouse() { mMouseIgnored = false; }
		void ignoreMouse() { mMouseIgnored = true; }
		Point2D mousePosition() { return mMouseIgnored? Point2D(InfFloat, InfFloat) : Point2D(mMouse.x, mMouse.y); }
		Point2D mouseMotion() { return mMouseIgnored? Point2D(0.0f, 0.0f) : (Point2D(mMouse.x, mMouse.y) - Point2D(mPrevMouse.x, mPrevMouse.y)); }
		bool mouseLeftPressed() { return mMouseIgnored? false : mMouse.left; }
		bool mouseLeftDown() { return mouseLeftPressed() && !mPrevMouse.left; }
		bool mouseLeftUp() { return !mouseLeftPressed() && mPrevMouse.left; }
		bool mouseLeftHeld() { return mouseLeftPressed() && mPrevMouse.left; }

		void update(const Window& win, const Point2D& pos, const Point2D& size);
		void render(const Window& win, const Point2D& pos, const Point2D& size) const;

	private:
		const Control* mFocus = nullptr;
		std::vector<const Control*> mTabIndex;
		MouseState mMouse, mPrevMouse;
		bool mMouseIgnored = false;

		void updateTabIndex(const Control* curr);
	};
	
	class ClipArea: public Control {
	public:
		ClipArea() = default;
		ClipArea(const Position& ul, const Position& lr, bool focusable = false): Control(ul, lr, focusable) {}
		
		void updateAll(const Point2D& parentPos, const Point2D& parentSize, Form& form) override;
		void renderAll(const Point2D& parentPos, const Point2D& parentSize, const Form& form, unsigned int channel = 0) const override;
	};

	class Label: public Control {
	public:
		Label() = default;
		Label(const Position& ul, const Position& lr, const std::string& text_ = "", bool focusable = false):
			Control(ul, lr, focusable), text(text_) {}
		std::string text = "";
		
	private:
		void render(const Point2D& ul, const Point2D& lr, const Form&) const override;
	};

	class Button: public Control {
	public:
		Button() = default;
		Button(const Position& ul, const Position& lr, const std::string& text_ = "", bool focusable = true):
			Control(ul, lr, focusable), text(text_) {}
		std::string text = "";

		bool mouseHover() const { return mHover; }
		bool pressed() const { return mPressed; }
		bool clicked() const { return mClicked; }

	private:
		bool mHover = false, mPressed = false, mClicked = false;

		void update(const Point2D& ul, const Point2D& lr, Form& form) override;
		void render(const Point2D& ul, const Point2D& lr, const Form& form) const override;
	};
	
	class TrackBar: public Control {
	public:
		TrackBar() = default;
		TrackBar(const Position& ul, const Position& lr, double lower_, double upper_, double value_,
				const std::string& text_ = "", bool focusable = true):
			Control(ul, lr, focusable), lower(lower_), upper(upper_), value(value_), text(text_) {}
		double lower = 0, upper = 0, value = 0;
		std::string text = "";

		bool mouseHover() const { return mHover; }
		bool modified() const { return mModified; }

	private:
		bool mHover = false, mButtonHover = false, mSelecting = false, mModified = false;

		void update(const Point2D& ul, const Point2D& lr, Form& form) override;
		void render(const Point2D& ul, const Point2D& lr, const Form& form) const override;
	};
	
	class PictureBox: public Control {
	public:
		PictureBox() = default;
		PictureBox(const Position& ul, const Position& lr, const Texture* picture_, bool focusable = true):
			Control(ul, lr, focusable), picture(picture_) {}
		const Texture* picture = nullptr;
		float borderWidth = 1.0f;

		bool mouseHover() const { return mHover; }
		bool pressed() const { return mPressed; }
		bool clicked() const { return mClicked; }

	private:
		bool mHover = false, mPressed = false, mClicked = false;

		void update(const Point2D& ul, const Point2D& lr, Form& form) override;
		void render(const Point2D& ul, const Point2D& lr, const Form& form) const override;
	};
	
	class HScroll: public Control {
	public:
		HScroll() = default;
		HScroll(const Position& ul, const Position& lr, double length_, double position_ = 0.0, double unit_ = 0.0, bool focusable = true):
			Control(ul, lr, focusable), length(length_), position(position_), unit(unit_) {}
		double length = 0, position = 0, unit = 0;

		bool mouseHover() const { return mHover; }
		bool modified() const { return mModified; }

	private:
		bool mHover = false, mSelecting = false, mModified = false;
		bool mLeftHover = false, mRightHover = false, mLeftPressed = false, mRightPressed = false;
		double mMouseOffset = 0;

		void update(const Point2D& ul, const Point2D& lr, Form& form) override;
		void render(const Point2D& ul, const Point2D& lr, const Form& form) const override;
	};
	
	class VScroll: public Control {
	public:
		VScroll() = default;
		VScroll(const Position& ul, const Position& lr, double length_, double position_ = 0.0, double unit_ = 0.0, bool focusable = true):
			Control(ul, lr, focusable), length(length_), position(position_), unit(unit_) {}
		double length = 0, position = 0, unit = 0;

		bool mouseHover() const { return mHover; }
		bool modified() const { return mModified; }

	private:
		bool mHover = false, mSelecting = false, mModified = false;
		bool mUpHover = false, mDownHover = false, mUpPressed = false, mDownPressed = false;
		double mMouseOffset = 0;

		void update(const Point2D& ul, const Point2D& lr, Form& form) override;
		void render(const Point2D& ul, const Point2D& lr, const Form& form) const override;
	};
	
	class ScrollArea: public Control {
	public:
		ScrollArea() = default;
		ScrollArea(const Position& ul, const Position& lr, const Point2D& size_, const Point2D& position_,
				bool draggable_ = false, bool scalable_ = false, bool focusable = false);
		Point2D size, position;
		float scale = 1.0f;
		bool draggable = false, scalable = false;

		void addChild(Control* c) override { mContent.addChild(c); }
		void addChild(std::initializer_list<Control*> c) override { mContent.addChild(c); }
		std::vector<Control*>& children() override { return mContent.children(); }
		
	private:
		ClipArea mContent, mView;
		HScroll mH;
		VScroll mV;
		bool mDragging = false, mScaling = false;
		
		void update(const Point2D& ul, const Point2D& lr, Form& form) override;
	};
}

#endif
