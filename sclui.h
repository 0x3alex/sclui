#pragma once
#define COLOR_BLANK -1
#include <vector>
#include <string>
#include <curses.h>
#include <assert.h>

namespace sclui {
    
    void initSclui();

    bool TextBoxFilterIsText(int c);

    bool TextBoxFilterIsNumber(int c);

    bool TextBoxFilterAllowAll(int c);

    void doQuit();

    class BasicItem {
        public:
            typedef enum {BASIC,BUTTON,CHECKBOX,TEXTBOX} types; 
            int itemIndex;
            types getType() const;
            std::string getName() const;
            int getX() const;
            int getY() const;
            void setX(int pX);
            void setY(int pY);
            bool isInteractable() const;
            void setInteractable(bool v);
            void moveTo();
            void setColor(int c);
            void setColorFocus(int c);
            bool isVisible() const;
            void setVisible(bool v);
            virtual void draw(bool v) = 0;
            void (*onDraw)() = nullptr;
            void (*onDestruct)() = nullptr;
        protected:
            std::string name = ("");
            int x,y,color, colorFocus;
            bool interactable,visible = true;
            types type;
            void chooseColor(bool b);
    };

    template <class T>
    class Interactable : public BasicItem{
        public:
            virtual T getValue() = 0;
    };

    class TextBox : public Interactable<std::string> {
        private:
            std::string value;
            int maxLength;
            char splitter;
        public:
            bool(*filter)(int) = nullptr;

            void(*onKeyPress)(int) = nullptr;

            void defaultKeyPressEvent(int c);

            TextBox(std::string pName,int px, int pY, int pMaxLength,int pColor, int pColorFocus, bool(*pFilter)(int), char pSplitter);
            virtual void draw(bool v) override;
            virtual std::string getValue() override;
            void setText(std::string s);
            int getValueLength() const;
            int getMaxLength() const;
            void append(char c);
            void pop();
    };

    class CheckBox : public Interactable<bool> {
        private:
            bool value;
        public:
            CheckBox(std::string pName,int px, int pY,int pColor, int pColorFocus, bool defaultValue);

            void(*onCheckBoxChange)() = nullptr;

            virtual void draw(bool v) override;
            virtual bool getValue() override;
            void setValue(bool v);
    };

    class Button : public Interactable<void> {   
        public:
            void(*onButtonPress)() = nullptr;

            Button(std::string pName,int px, int pY,int pColor, int pColorFocus);

            virtual void draw(bool v) override;
            virtual void getValue() override;
        };

        class Text : public BasicItem{
        public:
            Text(std::string pName,int px, int pY,int pColor);

            virtual void draw(bool v) override;
    };

    class Screen {
        private:
            std::string title;
            int width, height,vecIndex;
            BasicItem *currentItem;
            std::vector<BasicItem *> items = {};
            char hFrame, vFrame;
            void drawFrame();
            void drawItems();
            void run();
            void doMove(int mov);
            bool selectNext(BasicItem *i);
            BasicItem *getFirstInteractableItem();
        
        public:
            Screen(std::string pTitle, int pWidth, int pHeight, char pHFrame, char pVFrame);
            void addItem(BasicItem *i);

            BasicItem *getItemByName(const char *name);

            enum axis {X,Y,XY};

            void centerItem(Screen::axis pAxis, BasicItem *i);

            BasicItem *getItemAt(int index);

            void destroy();
            void draw();
            void update();
        };

}