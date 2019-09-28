#include "gui.h"
#include <algorithm>
#include "renderer.h"
#include "logger.h"
#include "textrenderer.h"

namespace GUI {
	// Colors (WHITE)
	/*
	const float ButtonColor0[4] = {0.0f, 0.6f, 1.0f, 1.0f};
	const float ButtonColor1[4] = {0.5f, 0.8f, 1.0f, 1.0f};
	const float BackColor0[4] = {0.8f, 0.8f, 0.8f, 0.6f};
	const float BackColor1[4] = {0.8f, 0.8f, 0.8f, 0.4f};
	const float TextColor[4] = {0.2f, 0.2f, 0.2f, 1.0f};
	*/
	// Colors (DARK)
//	/*
	const float ButtonColor0[4] = {0.0f, 0.6f, 1.0f, 1.0f};
	const float ButtonColor1[4] = {0.0f, 0.4f, 0.7f, 1.0f};
	const float BackColor0[4] = {0.3f, 0.3f, 0.3f, 0.6f};
	const float BackColor1[4] = {0.3f, 0.3f, 0.3f, 0.4f};
	const float TextColor[4] = {0.9f, 0.9f, 0.9f, 1.0f};
//	*/
	// Sizes
	const int TrackBarWidth = 10;
	const int DefaultHScrollWidth = 18;
	const int DefaultVScrollWidth = 18;
	const int ScrollButtonSize = 18;
	const int MinScrollLength = 20;
	const double DefaultUnitFraction = 0.1;
	const int LineWidth = 2;
	
	// Add a quad to vertex array
	inline void drawQuad(VertexArray& va, float x0, float y0, float x1, float y1) {
		va.addVertex({ float(x0), float(y0) });
		va.addVertex({ float(x0), float(y1) });
		va.addVertex({ float(x1), float(y0) });
		va.addVertex({ float(x1), float(y0) });
		va.addVertex({ float(x0), float(y1) });
		va.addVertex({ float(x1), float(y1) });
	}

	// Draw string using TextRenderer::drawAscii
	inline void drawTextCentered(const Point2D& ul, const Point2D& lr, const std::string& s) {
		int len = s.size(), width = 6, height = 10;
		Vec3f pos((ul.x + lr.x - len * width) / 2.0f, (ul.y + lr.y - height) / 2.0f, 0.0f);
		Renderer::enableTexture2D();
		TextRenderer::drawAscii(pos, s, height, false, TextColor);
		Renderer::disableTexture2D();
	}
	
	inline void drawColorTextCentered(const Point2D& ul, const Point2D& lr, const std::string& s, const Vec3f& col) {
		int len = s.size(), width = 6, height = 10;
		Vec3f pos((ul.x + lr.x - len * width) / 2.0f, (ul.y + lr.y - height) / 2.0f, 0.0f);
		Renderer::enableTexture2D();
		TextRenderer::drawAscii(pos, s, height, false, col);
		Renderer::disableTexture2D();
	}

	bool Control::focused(const Form& form) const { return focusable && this == form.focus(); }
	void Control::getFocus(Form& form) const { if (focusable) form.setFocus(this); }

	void Form::updateTabIndex(const Control* curr) { // DFS
		if (curr->focusable) mTabIndex.push_back(curr);
		for (const Control* c: curr->realChildren()) updateTabIndex(c);
	}
	
	void Form::update(const Window& win, const Point2D& pos, const Point2D& size) {
		// Get input
		mMouse = win.getMouseState();
		mPrevMouse = win.getPrevMouseState();
		// Tab index & focus
		mTabIndex.clear();
		updateTabIndex(mArea);
		if (!mTabIndex.empty()) {
			int n = mTabIndex.size(), ind = std::find(mTabIndex.begin(), mTabIndex.end(), mFocus) - mTabIndex.begin();
			if (win.isKeyActed(SDL_SCANCODE_TAB)) {
				if (win.isKeyPressed(SDL_SCANCODE_LSHIFT) || win.isKeyPressed(SDL_SCANCODE_RSHIFT)) --ind;
				else ++ind;
			}
			ind = (ind % n + n) % n;
			mFocus = mTabIndex[ind];
		} else mFocus = nullptr;
		// Update all controls
		mArea->updateAll(pos, size, *this);
	}
	
	void Form::render(const Window&, const Point2D& pos, const Point2D& size) const {
		mArea->renderAll(pos, size, *this);
	}
	
	void ClipArea::updateAll(const Point2D& parentPos, const Point2D& parentSize, Form& form) {
		Point2D ul = upperLeft.compute(parentSize) + parentPos;
		Point2D lr = lowerRight.compute(parentSize) + parentPos;
		if (active) {
			bool mouseIn = (form.mousePosition() >= ul && form.mousePosition() <= lr), ignored = form.mouseIgnored();
			if (!mouseIn) form.ignoreMouse();
			for (Control* c: realChildren()) c->updateAll(ul, lr - ul, form);
			if (!mouseIn && !ignored) form.awareMouse();
		}
	}
	
	void ClipArea::renderAll(const Point2D& parentPos, const Point2D& parentSize, const Form& form, unsigned int channel) const {
		Point2D ul = upperLeft.compute(parentSize) + parentPos;
		Point2D lr = lowerRight.compute(parentSize) + parentPos;
		if (active) {
			VertexArray va(120, VertexFormat(0, 4, 0, 3));
			va.setColor({ 0.0f, 0.0f, 0.0f, 0.0f });
			drawQuad(va, ul.x, ul.y, lr.x, lr.y);
			VertexBuffer vb(va);
			glStencilFunc(GL_EQUAL, channel, 0xFF); // (This should have been done)
			glStencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP);
			vb.render(); // Initialize clip area
			glStencilFunc(GL_EQUAL, channel + 1, 0xFF);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
			for (const Control* c: realChildren()) c->renderAll(ul, lr - ul, form, channel + 1);
			glStencilFunc(GL_EQUAL, channel + 1, 0xFF); // (This should have been done)
			glStencilOp(GL_KEEP, GL_KEEP, GL_DECR_WRAP);
			vb.render(); // Discard clip area
			glStencilFunc(GL_EQUAL, channel, 0xFF);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		}
	}
	
	void Label::render(const Point2D& ul, const Point2D& lr, const Form&) const {
		// Text
		drawTextCentered(ul, lr, text);
	}

	void Button::update(const Point2D& ul, const Point2D& lr, Form& form) {
		mHover = mPressed = mClicked = false;
		if (focusable && form.mousePosition() >= ul && form.mousePosition() <= lr) {
			mHover = true;
			if (form.mouseLeftDown()) getFocus(form);
			if (form.mouseLeftPressed()) mPressed = true;
			if (focused(form) && form.mouseLeftUp()) mClicked = true;
		}
	}

	void Button::render(const Point2D& ul, const Point2D& lr, const Form& form) const {
		VertexArray va(120, VertexFormat(0, 4, 0, 3));
		// Border
		va.setColor(4, mHover? ButtonColor1 : ButtonColor0);
		drawQuad(va, ul.x, ul.y, lr.x, lr.y);
		// Background quad
		va.setColor(4, mPressed? ButtonColor1 : ButtonColor0);
		drawQuad(va, ul.x + LineWidth, ul.y + LineWidth, lr.x - LineWidth, lr.y - LineWidth);
		VertexBuffer(va).render();
		// Text
		drawColorTextCentered(ul, lr, text, Vec3f(1.0f, 1.0f, 1.0f));
	}

	void TrackBar::update(const Point2D& ul, const Point2D& lr, Form& form) {
		mHover = mButtonHover = mModified = false;
		// Begin selection
		if (focusable && form.mousePosition() >= ul && form.mousePosition() <= lr) {
			mHover = true;
			if (form.mouseLeftDown()) getFocus(form), mSelecting = true;
			/*
			int hw = TrackBarWidth / 2, xmin = ul.x + hw, xmax = lr.x - hw;
			int xsel = (value - lower) / (upper - lower) * (xmax - xmin) + xmin;
			if (form.mousePosition().x >= xsel - hw && form.mousePosition().x <= xsel + hw) mButtonHover = true;
			*/
		}
		// Update selection
		if (form.mouseLeftPressed() && mSelecting) {
			int hw = TrackBarWidth / 2, xmin = ul.x + hw, xmax = lr.x - hw, xsel = form.mousePosition().x;
			xsel = std::min(std::max(xsel, xmin), xmax);
			double newValue = (xsel - xmin) / double(xmax - xmin) * (upper - lower) + lower;
			mModified = (newValue != value);
			value = newValue;
		} else mSelecting = false; // Released left button
	}

	void TrackBar::render(const Point2D& ul, const Point2D& lr, const Form& form) const {
		int hw = TrackBarWidth / 2, xmin = ul.x + hw, xmax = lr.x - hw;
		int xsel = (value - lower) / (upper - lower) * (xmax - xmin) + xmin;
		VertexArray va(120, VertexFormat(0, 4, 0, 3));
		// Background quad
		va.setColor(4, mSelecting? BackColor1 : (mHover? BackColor0 : BackColor1));
		drawQuad(va, ul.x, ul.y + LineWidth, lr.x, lr.y - LineWidth);
		// Foreground Quad
		va.setColor(4, (mButtonHover || mSelecting)? ButtonColor1 : ButtonColor0);
		drawQuad(va, float(xsel - hw), ul.y, float(xsel + hw), lr.y);
		va.setColor(4, mSelecting? ButtonColor1 : ButtonColor0);
		drawQuad(va, float(xsel - hw) + LineWidth, ul.y + LineWidth, float(xsel + hw) - LineWidth, lr.y - LineWidth);
		VertexBuffer(va).render();
		// Text
		drawTextCentered(ul, lr, text);
	}

	void PictureBox::update(const Point2D& ul, const Point2D& lr, Form& form) {
		mHover = mPressed = mClicked = false;
		if (focusable && form.mousePosition() >= ul && form.mousePosition() <= lr) {
			mHover = true;
			if (form.mouseLeftDown()) getFocus(form);
			if (form.mouseLeftPressed()) mPressed = true;
			if (focused(form) && form.mouseLeftUp()) mClicked = true;
		}
	}

	void PictureBox::render(const Point2D& ul, const Point2D& lr, const Form& form) const {
		VertexArray va(120, VertexFormat(0, 4, 0, 3)), tva(120, VertexFormat(2, 4, 0, 3));
		// Background quad
		va.setColor(4, mPressed? BackColor1 : (mHover? BackColor0 : BackColor1));
		drawQuad(va, ul.x, ul.y, lr.x, lr.y);
		VertexBuffer(va).render();
		// Picture
		if (picture != nullptr) {
			tva.setColor({ 1.0f, 1.0f, 1.0f, 1.0f });
			tva.setTexture({ 0.0f, 0.0f }); tva.addVertex({ ul.x + borderWidth, ul.y + borderWidth });
			tva.setTexture({ 0.0f, 1.0f }); tva.addVertex({ ul.x + borderWidth, lr.y - borderWidth });
			tva.setTexture({ 1.0f, 0.0f }); tva.addVertex({ lr.x - borderWidth, ul.y + borderWidth });
			tva.setTexture({ 0.0f, 1.0f }); tva.addVertex({ ul.x + borderWidth, lr.y - borderWidth });
			tva.setTexture({ 1.0f, 1.0f }); tva.addVertex({ lr.x - borderWidth, lr.y - borderWidth });
			tva.setTexture({ 1.0f, 0.0f }); tva.addVertex({ lr.x - borderWidth, ul.y + borderWidth });
			Renderer::enableTexture2D();
			picture->bind();
			VertexBuffer(tva).render();
			Renderer::disableTexture2D();
		}
	}
	
	void HScroll::update(const Point2D& ul, const Point2D& lr, Form& form) {
		mHover = mModified = false;
		mLeftHover = mLeftPressed = mRightHover = mRightPressed = false;
		double oldPosition = position;
		// Areas
		Point2D ulsa = ul + Point2D(ScrollButtonSize, 0), lrsa = lr - Point2D(ScrollButtonSize, 0); // Scroll area
		float len = std::max((lrsa.x - ulsa.x) * float(length), float(MinScrollLength));
		float left = ulsa.x + (lrsa.x - ulsa.x - len) * position;
		Point2D uls = Point2D(left, ulsa.y), lrs = Point2D(left + len, lrsa.y); // Scroll bar
		Point2D ulleft = ul, lrleft = Point2D(ul.x + ScrollButtonSize, lr.y); // Left button
		Point2D ulright = Point2D(lr.x - ScrollButtonSize, ul.y), lrright = lr; // Right button
		// Begin selection
		if (focusable && form.mousePosition() >= uls && form.mousePosition() <= lrs) {
			mHover = true;
			if (form.mouseLeftDown()) {
				getFocus(form), mSelecting = true;
				mMouseOffset = form.mousePosition().x - left;
			}
		}
		// Update selection
		if (form.mouseLeftHeld() && mSelecting) {
			left = form.mousePosition().x - mMouseOffset;
			if (lrsa.x - ulsa.x - len > 0.0) position = (left - ulsa.x) / (lrsa.x - ulsa.x - len);
		}
		if (!form.mouseLeftPressed()) mSelecting = false;
		// Update buttons
		double delta = (unit > 0.0? unit : length * DefaultUnitFraction);
		if (focusable && form.mousePosition() >= ulleft && form.mousePosition() <= lrleft) {
			mLeftHover = true;
			if (form.mouseLeftDown()) {
				getFocus(form);
				if (length < 1.0) position -= delta / (1.0 - length);
			}
			if (form.mouseLeftPressed()) mLeftPressed = true;
		}
		if (focusable && form.mousePosition() >= ulright && form.mousePosition() <= lrright) {
			mRightHover = true;
			if (form.mouseLeftDown()) {
				getFocus(form);
				if (length < 1.0) position += delta / (1.0 - length);
			}
			if (form.mouseLeftPressed()) mRightPressed = true;
		}
		position = std::min(std::max(position, 0.0), 1.0);
		mModified = (position != oldPosition);
	}

	void HScroll::render(const Point2D& ul, const Point2D& lr, const Form& form) const {
		// Areas
		Point2D ulsa = ul + Point2D(ScrollButtonSize, 0), lrsa = lr - Point2D(ScrollButtonSize, 0); // Scroll area
		float len = std::max((lrsa.x - ulsa.x) * float(length), float(MinScrollLength));
		float left = ulsa.x + (lrsa.x - ulsa.x - len) * position;
		Point2D uls = Point2D(left, ulsa.y), lrs = Point2D(left + len, lrsa.y); // Scroll bar
		Point2D ulleft = ul, lrleft = Point2D(ul.x + ScrollButtonSize, lr.y); // Left button
		Point2D ulright = Point2D(lr.x - ScrollButtonSize, ul.y), lrright = lr; // Right button
		// Render
		VertexArray va(120, VertexFormat(0, 4, 0, 3));
		// Background quad
		va.setColor(4, BackColor1);
		drawQuad(va, ul.x, ul.y, lr.x, lr.y);
		// Border
		va.setColor(4, (mHover || mSelecting)? ButtonColor1 : ButtonColor0);
		drawQuad(va, float(left), ul.y, float(left + len), lr.y);
		// Foreground quad
		va.setColor(4, mSelecting? ButtonColor1 : ButtonColor0);
		drawQuad(va, float(left) + LineWidth, ul.y + LineWidth, float(left + len) - LineWidth, lr.y - LineWidth);
		// Left Button
		if (mLeftHover) {
			va.setColor(4, BackColor1);
			drawQuad(va, ulleft.x, ulleft.y, lrleft.x, lrleft.y);
		}
		Point2D center = ((ulleft + lrleft) / 2.0f).round() + Point2D(0.0f, 0.0f);
		va.setColor(4, mLeftPressed? ButtonColor1 : ButtonColor0);
		va.addVertex({ center.x - 3, center.y + 1 });
		va.addVertex({ center.x, center.y + 1 });
		va.addVertex({ center.x + 4, center.y + 1 - 4 });
		va.addVertex({ center.x - 3, center.y + 1 });
		va.addVertex({ center.x + 4, center.y + 1 - 4 });
		va.addVertex({ center.x - 3 + 4, center.y + 1 - 4 });
		va.addVertex({ center.x, center.y });
		va.addVertex({ center.x - 3, center.y });
		va.addVertex({ center.x + 4, center.y + 4 });
		va.addVertex({ center.x + 4, center.y + 4 });
		va.addVertex({ center.x - 3, center.y });
		va.addVertex({ center.x - 3 + 4, center.y + 4 });
		// Right Button
		if (mRightHover) {
			va.setColor(4, BackColor1);
			drawQuad(va, ulright.x, ulright.y, lrright.x, lrright.y);
		}
		center = ((ulright + lrright) / 2.0f).round() + Point2D(1.0f, 0.0f);
		va.setColor(4, mRightPressed? ButtonColor1 : ButtonColor0);
		va.addVertex({ center.x, center.y + 1 });
		va.addVertex({ center.x + 3, center.y + 1 });
		va.addVertex({ center.x - 4, center.y + 1 - 4 });
		va.addVertex({ center.x - 4, center.y + 1 - 4 });
		va.addVertex({ center.x + 3, center.y + 1 });
		va.addVertex({ center.x + 3 - 4, center.y + 1 - 4 });
		va.addVertex({ center.x + 3, center.y });
		va.addVertex({ center.x, center.y });
		va.addVertex({ center.x - 4, center.y + 4 });
		va.addVertex({ center.x + 3, center.y });
		va.addVertex({ center.x - 4, center.y + 4 });
		va.addVertex({ center.x + 3 - 4, center.y + 4 });
		VertexBuffer(va).render();
	}
	
	void VScroll::update(const Point2D& ul, const Point2D& lr, Form& form) {
		mHover = mModified = false;
		mUpHover = mUpPressed = mDownHover = mDownPressed = false;
		double oldPosition = position;
		// Areas
		Point2D ulsa = ul + Point2D(0, ScrollButtonSize), lrsa = lr - Point2D(0, ScrollButtonSize); // Scroll area
		float len = std::max((lrsa.y - ulsa.y) * float(length), float(MinScrollLength));
		float up = ulsa.y + (lrsa.y - ulsa.y - len) * position;
		Point2D uls = Point2D(ulsa.x, up), lrs = Point2D(lrsa.x, up + len); // Scroll bar
		Point2D ulup = ul, lrup = Point2D(lr.x, ul.y + ScrollButtonSize); // Up button
		Point2D uldown = Point2D(ul.x, lr.y - ScrollButtonSize), lrdown = lr; // Down button
		// Begin selection
		if (focusable && form.mousePosition() >= uls && form.mousePosition() <= lrs) {
			mHover = true;
			if (form.mouseLeftDown()) {
				getFocus(form), mSelecting = true;
				mMouseOffset = form.mousePosition().y - up;
			}
		}
		// Update selection
		if (form.mouseLeftHeld() && mSelecting) {
			up = form.mousePosition().y - mMouseOffset;
			if (lrsa.y - ulsa.y - len > 0.0) position = (up - ulsa.y) / (lrsa.y - ulsa.y - len);
		}
		if (!form.mouseLeftPressed()) mSelecting = false;
		// Update buttons
		double delta = (unit > 0.0? unit : length * DefaultUnitFraction);
		if (focusable && form.mousePosition() >= ulup && form.mousePosition() <= lrup) {
			mUpHover = true;
			if (form.mouseLeftDown()) {
				getFocus(form);
				if (length < 1.0) position -= delta / (1.0 - length);
			}
			if (form.mouseLeftPressed()) mUpPressed = true;
		}
		if (focusable && form.mousePosition() >= uldown && form.mousePosition() <= lrdown) {
			mDownHover = true;
			if (form.mouseLeftDown()) {
				getFocus(form);
				if (length < 1.0) position += delta / (1.0 - length);
			}
			if (form.mouseLeftPressed()) mDownPressed = true;
		}
		position = std::min(std::max(position, 0.0), 1.0);
		mModified = (position != oldPosition);
	}

	void VScroll::render(const Point2D& ul, const Point2D& lr, const Form& form) const {
		// Areas
		Point2D ulsa = ul + Point2D(0, ScrollButtonSize), lrsa = lr - Point2D(0, ScrollButtonSize); // Scroll area
		float len = std::max((lrsa.y - ulsa.y) * float(length), float(MinScrollLength));
		float up = ulsa.y + (lrsa.y - ulsa.y - len) * position;
		Point2D uls = Point2D(ulsa.x, up), lrs = Point2D(lrsa.x, up + len); // Scroll bar
		Point2D ulup = ul, lrup = Point2D(lr.x, ul.y + ScrollButtonSize); // Up button
		Point2D uldown = Point2D(ul.x, lr.y - ScrollButtonSize), lrdown = lr; // Down button
		// Render
		VertexArray va(120, VertexFormat(0, 4, 0, 3));
		// Background quad
		va.setColor(4, BackColor1);
		drawQuad(va, ul.x, ul.y, lr.x, lr.y);
		// Border
		va.setColor(4, (mHover || mSelecting)? ButtonColor1 : ButtonColor0);
		drawQuad(va, ul.x, float(up), lr.x, float(up + len));
		// Foreground quad
		va.setColor(4, mSelecting? ButtonColor1 : ButtonColor0);
		drawQuad(va, ul.x + LineWidth, float(up) + LineWidth, lr.x - LineWidth, float(up + len) - LineWidth);
		// Up Button
		if (mUpHover) {
			va.setColor(4, BackColor1);
			drawQuad(va, ulup.x, ulup.y, lrup.x, lrup.y);
		}
		Point2D center = ((ulup + lrup) / 2.0f).round() + Point2D(0.5f, -0.5f);
		va.setColor(4, mUpPressed? ButtonColor1 : ButtonColor0);
		va.addVertex({ center.x + 1, center.y });
		va.addVertex({ center.x + 1, center.y - 3 });
		va.addVertex({ center.x + 1 - 4, center.y + 4 });
		va.addVertex({ center.x + 1 - 4, center.y + 4 });
		va.addVertex({ center.x + 1, center.y - 3 });
		va.addVertex({ center.x + 1 - 4, center.y - 3 + 4 });
		va.addVertex({ center.x, center.y - 3 });
		va.addVertex({ center.x, center.y });
		va.addVertex({ center.x + 4, center.y + 4 });
		va.addVertex({ center.x, center.y - 3 });
		va.addVertex({ center.x + 4, center.y + 4 });
		va.addVertex({ center.x + 4, center.y - 3 + 4 });
		// Down Button
		if (mDownHover) {
			va.setColor(4, BackColor1);
			drawQuad(va, uldown.x, uldown.y, lrdown.x, lrdown.y);
		}
		center = ((uldown + lrdown) / 2.0f).round() + Point2D(0.5f, 0.5f);
		va.setColor(4, mDownPressed? ButtonColor1 : ButtonColor0);
		va.addVertex({ center.x + 1, center.y + 3 });
		va.addVertex({ center.x + 1, center.y });
		va.addVertex({ center.x + 1 - 4, center.y - 4 });
		va.addVertex({ center.x + 1, center.y + 3 });
		va.addVertex({ center.x + 1 - 4, center.y - 4 });
		va.addVertex({ center.x + 1 - 4, center.y + 3 - 4 });
		va.addVertex({ center.x, center.y });
		va.addVertex({ center.x, center.y + 3 });
		va.addVertex({ center.x + 4, center.y - 4 });
		va.addVertex({ center.x + 4, center.y - 4 });
		va.addVertex({ center.x, center.y + 3 });
		va.addVertex({ center.x + 4, center.y + 3 - 4 });
		VertexBuffer(va).render();
	}
	
	ScrollArea::ScrollArea(const Position& ul, const Position& lr, const Point2D& size_, const Point2D& position_, bool draggable_, bool scalable_, bool focusable):
			Control(ul, lr, focusable), size(size_), position(position_), draggable(draggable_), scalable(scalable_),
			mContent(Position(Point2D(0.5f, 0.5f), Point2D(0, 0)), Position(Point2D(0.5f, 0.5f), Point2D(0, 0))),
			mView(Position(Point2D(0.0f, 0.0f), Point2D(0, 0)), Position(Point2D(1.0f, 1.0f), Point2D(0, 0))),
			mH(Position(Point2D(0.0f, 1.0f), Point2D(0, -DefaultHScrollWidth)), Position(Point2D(1.0f, 1.0f), Point2D(0, 0)), 1.0),
			mV(Position(Point2D(1.0f, 0.0f), Point2D(-DefaultVScrollWidth, 0)), Position(Point2D(1.0f, 1.0f), Point2D(0, 0)), 1.0) {
		mView.addChild({&mContent});
		Control::addChild({&mView, &mH, &mV});
	}
	
	void ScrollArea::update(const Point2D& ul, const Point2D& lr, Form&) {
		mH.active = mV.active = false;
		mView.lowerRight.offset = Point2D(0, 0); // Default: no scroll bars
		Point2D vsize = mView.lowerRight.compute(lr - ul) - mView.upperLeft.compute(lr - ul); // Viewport size
		if (size.x * scale > vsize.x) mH.active = true, mView.lowerRight.offset.y -= DefaultHScrollWidth, vsize.y -= DefaultHScrollWidth; // Enable HScroll
		if (size.y * scale > vsize.y) mV.active = true, mView.lowerRight.offset.x -= DefaultVScrollWidth, vsize.x -= DefaultVScrollWidth; // Enable VScroll
		if (size.x * scale > vsize.x && !mH.active) mH.active = true, mView.lowerRight.offset.y -= DefaultHScrollWidth, vsize.y -= DefaultHScrollWidth; // Enable HScroll
		if (mH.active && mV.active) { // Both enabled
			mH.lowerRight.offset.x = -DefaultVScrollWidth - 1;
			mV.lowerRight.offset.y = -DefaultHScrollWidth - 1;
		} else {
			mH.lowerRight.offset.x = 0;
			mV.lowerRight.offset.y = 0;
		}
		// Calculate content area offset
		position.x = mH.position, position.y = mV.position;
		Point2D offset = size * scale - vsize;
		offset.x = std::max(0.0f, offset.x), offset.y = std::max(0.0f, offset.y);
		offset = offset * (position - Point2D(0.5f, 0.5f));
		mContent.upperLeft.offset = offset * -1.0f - size * scale / 2.0f + mView.lowerRight.offset / 2.0f;
		mContent.lowerRight.offset = offset * -1.0f + size * scale / 2.0f + mView.lowerRight.offset / 2.0f;
		mView.lowerRight.offset = Point2D(0, 0);
		// TODO: implement dragging & scaling
		Point2D fraction = vsize / size;
		fraction.x = std::min(1.0f, fraction.x), fraction.y = std::min(1.0f, fraction.y);
		mH.length = fraction.x, mV.length = fraction.y;
	}
}

