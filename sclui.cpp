#include "sclui.hpp"
#include <string>
#include <string_view>
#include <curses.h>


namespace sclui {
    #ifndef SWITCH_KEY
    #define SWITCH_KEY '\t'
    #endif

    #ifndef CONFIRM_KEY
    #define CONFIRM_KEY '\n'
    #endif

    #ifndef DRAG_KEY
    #define DRAG_KEY '#'
    #endif

    #ifndef UP_KEY
    #define UP_KEY KEY_UP
    #endif

    #ifndef DOWN_KEY
    #define DOWN_KEY KEY_DOWN
    #endif

    #ifndef H_FRAME
    #define H_FRAME '-'
    #endif

    #ifndef V_FRAME
    #define V_FRAME '|'
    #endif

    #ifndef H_FRAME_FOCUS
    #define H_FRAME_FOCUS '='
    #endif

    #ifndef H_FRAME_DRAG
    #define H_FRAME_DRAG '+'
    #endif

    static int cIndex = 2;

    static Screen *currentScreen = nullptr;

    void doQuit() {
        curs_set(1);
        endwin();
        exit(0);
    }

    int getMaxX() {
        return getmaxx(stdscr);
    }
    int getMaxY() {
        return getmaxy(stdscr);
    }


    void initSclui() {
        WINDOW *w = initscr();
        assert(w != nullptr);
        raw();
        noecho();
        start_color();
        keypad(stdscr,TRUE);
        clear();
    }

    bool TextBoxFilterIsText(int c) {
        return isalpha(c) || c == ' ';
    }

    bool TextBoxFilterIsNumber(int c) {
        return isdigit(c);
    }

    bool TextBoxFilterAllowAll(int c) {
        return true;
    }

    BasicItem::types BasicItem::getType() const {
        return type;
    }

    bool BasicItem::isVisible() const {
        return visible;
    }
    void BasicItem::setVisible(bool v) {
        visible = v;
    }

    void BasicItem::setColor(int c) {
        color = c;
    }
    void BasicItem::setColorFocus(int c) {
        colorFocus = c;
    }

    int BasicItem::getX() const {
        return x;
    }
    int BasicItem::getY() const {
        return y;
    }

    void BasicItem::setX(int pX) {
        x = pX;
    }
    void BasicItem::setY(int pY) {
        y = pY;
    }

    bool BasicItem::isInteractable() const {
        return interactable;
    }
    
    void BasicItem::setInteractable(bool v) {
        interactable = v;
    }

    void BasicItem::moveTo() {
        move(currentScreen->y + y,currentScreen->x + x);
    }

    void BasicItem::chooseColor(bool b) {
        attroff(COLOR_PAIR(itemIndex));
        if(color == COLOR_BLANK) return;
        init_pair(itemIndex,COLOR_BLACK, b ? colorFocus : color);
        attron(COLOR_PAIR(itemIndex));
    }


    Button::Button(std::string_view  name, int x, int y, 
    int color, int colorFocus) {
        name = name;
        type = BUTTON;
        color = color;
        colorFocus = colorFocus;
        itemIndex = cIndex++;
        interactable = true;
        x = x;
        y = y;
    }

    void Button::draw(bool v) {
        chooseColor(v);
        curs_set(0);
        this->moveTo();
        printw("[ %s ]" , name.c_str()); 
        this->moveTo();
    }

    void *Button::getValue() {
        return nullptr;
    }

    CheckBox::CheckBox(std::string_view  name,int x, int y,int color, 
    int colorFocus, bool value) {
        name = name;
        x = x;
        y = y;
        color = color;
        colorFocus = colorFocus;
        value = value;
        interactable = true;
        type = CHECKBOX;
        itemIndex = cIndex++;
    }

    void CheckBox::draw(bool v) {
        chooseColor(v);
        curs_set(0);
        this->moveTo();
        printw("[ %s (%c) ]", name.c_str(), value == true ? 'X' : ' ');
        this->moveTo();
    }

    bool *CheckBox::getValue() {
        return &value;
    }

    void CheckBox::setValue(bool v) {
        value = v;
    }

    Text::Text(std::string_view  name,int x, int y,int color) {
        name = name;
        x = x;
        y = y;
        color = color;
        interactable = false;
        type = BASIC;
        itemIndex = cIndex++;

    }
    void Text::draw(bool v) {
        int oldY = this->getY();
        chooseColor(v);
        curs_set(0);
        this->moveTo();
        for(char n : name) {
            if(n == '\n') {
                this->setY(this->getY() + 1);
                this->moveTo();
                continue;
            }
            printw("%c",n);
        }
        this->setY(oldY);
        this->moveTo();
    }

    //screen
    Screen::Screen(std::string_view title, int width, int height, int x, int y) : 
                title{title},
                width{width},
                height{height},
                x{x},
                y{y}
                {}


    void Screen::addItem(BasicItem *i) {
        items.push_back(i);
    }

    BasicItem *Screen::getFirstInteractableItem() {
        if(items.size() == 0) return nullptr;
        int index = 0;
        for(auto &i: items) {
            if(i->isInteractable() && i->isVisible()) {
                vecIndex = index;
                return i;
            }
            index++;
            
        }
        return nullptr;
    }

    BasicItem *Screen::getItemByName(const char *name) {
        //auto generate type
        for(auto &i : items) {
            if(i->name.compare(name)== 0) {
                return i;
            }
        }
        return nullptr;
    }

    TextBox::TextBox(std::string_view  name,int x, int y, int maxLength,int color, 
    int colorFocus, bool(*filter)(int), char splitter) {
        name = name;
        x = x;
        y = y;
        maxLength = maxLength;
        itemIndex = cIndex++;
        interactable = true;
        type = TEXTBOX;
        color = color;
        colorFocus = colorFocus;
        filter = filter;
        splitter = splitter;
    }

    void TextBox::defaultKeyPressEvent(int c) {
        if(c == KEY_BACKSPACE && this->value.size() > 0) {
            this->pop();
        }else{
            if(this->value.size() < this->maxLength && (*(this->filter))(c)) {
                this->append(c);
            }
        }
        if(this->onDraw != nullptr) (*(this->onDraw))();
        this->draw(true);
    }

    void TextBox::draw(bool v) {
        chooseColor(v);
        curs_set(1);
        this->moveTo();
        printw("[ %s%c" , name.c_str(),splitter); 
        for(int i = 0; i < maxLength; i++) printw(" ");
        printw("]");
        move(currentScreen->y+ y, currentScreen->x + x+3 + name.length());
        printw("%s",value.c_str());
    }

    std::string *TextBox::getValue() {
        return &value;
    }
    void TextBox::append(char c) {
        value.push_back(c);
    }
    void TextBox::pop() {
        value.pop_back();
    }

    char getHFrame(int v) {
        /*
            v = 0 is normal
            v = 1 is focus
            v = 2 is drag
        */
        if(v == 0) {
            return H_FRAME;
        }else if (v == 1)
        {
            return H_FRAME_FOCUS;
        }else{
            return H_FRAME_DRAG;
        }
        
    }

    void Screen::drawFrame(int v) {
        move(y,x);
        init_pair(1,COLOR_BLACK,COLOR_WHITE);
        attron(COLOR_PAIR(1));
        for(int i = 0; i <= width; i++) {
            printw("%c",getHFrame(v));
            if(i == 2) {
                addstr(title.c_str());
                i += title.length();
            }
        }
        move(y+height,x);
        for(int i = 0; i <= width; i++) {
            printw("%c",getHFrame(v));
        }
        for(int i = 0; i <= height; i++) {
            move(y + i, x);
            printw("%c",V_FRAME);
        }
        for(int i = 0; i <= height; i++) {
            move(y + i, x + width);
            printw("%c",V_FRAME);
        }
        attroff(COLOR_PAIR(1));
    }

    bool Screen::selectNext(BasicItem *i) {
        if(!i->isInteractable() || !i->isVisible()) return false;
        if(i->onDraw != nullptr) (*(i->onDraw))();
        i->draw(true);
        return true;
    }

    void Screen::moveHelper(int i) {
        if(currentItem->onDraw != nullptr) (*(currentItem->onDraw))();
        currentItem->draw(false);
        vecIndex = i;
        currentItem = items.at(i);
        if(currentItem->onDraw != nullptr) (*(currentItem->onDraw))();
        currentItem->draw(true);
    }

    void Screen::doMove(int mov) {
        if(mov == 0)  { //down
            for(int i = vecIndex+1; i < items.size(); i++) {
                if(selectNext(items.at(i))) {
                    moveHelper(i);
                    return;
                }
            }
        }else{ //up
            for(int i = vecIndex-1; i >= 0; i--) {
                if(selectNext(items.at(i))) {
                    moveHelper(i);
                    return;
                }
            } 
        }
        refresh();
    }

    void Screen::drawItems() {
        for(auto &i : items) {
            if(i->isVisible()) {
                if(i->onDraw != nullptr) (*(i->onDraw))();
                i->draw(false);
            }
                
        }
        vecIndex = 0;
        currentItem = getFirstInteractableItem();
        refresh();
        
    }


    void drag(int c) {
        int fullX = currentScreen->x+ currentScreen->width;
        int fullY = currentScreen->y + currentScreen->height;
        int x = currentScreen->x;
        int y = currentScreen->y;
        Screen *m = currentScreen->motherScreen;
        switch(c) {
            case KEY_RIGHT:
                if(fullX+1 < m->width) {
                    currentScreen->x++;
                }
                break;
            case KEY_LEFT:
                if(x-1 >= 0) {
                    currentScreen->x--;
                }
                break;
            case KEY_UP:
                if(y-1 >= 0) {
                    currentScreen->y--;
                }
                break;
            case KEY_DOWN:
                if(fullY+1 < m->height) {
                    currentScreen->y++;
                }
                break;
        }
        m->update(); //just update
        
    }

    void Screen::handleDrag(int c) {
        if(currentScreen->motherScreen != nullptr) {
            currentScreen->isDragging = true;
            if(currentScreen->onDragBegin != nullptr) (*(currentScreen->onDragBegin))();
            do{
                if(currentScreen->onDrag != nullptr) (*(currentScreen->onDrag))();
                drag(c);
            }while((c=getch()) != DRAG_KEY);
            currentScreen->isDragging = false;
            if(currentScreen->onDrop != nullptr) (*(currentScreen->onDrop))();
            currentScreen->motherScreen->draw(); //redraw all after dragging is done
        } 
    }

    void Screen::switchFocus() {
        if(currentScreen->motherScreen->subScreenIndex 
        == currentScreen->motherScreen->subScreens.size()-1) {
            currentScreen->motherScreen->subScreenIndex = 0;
        }else{
            currentScreen->motherScreen->subScreenIndex++;
        }
        //redraw to ensure that item interactions are working correctly
        currentScreen->motherScreen->draw();
    }
    void Screen::run() {
        int c;
        CheckBox *cCast = nullptr;
        Button *bCast = nullptr;
        TextBox * tbCast = nullptr;
        for(;;) {
            c = getch();
            switch(c) {
                case DRAG_KEY:
                    handleDrag(c);
                    break;
                case SWITCH_KEY:
                    //allow switching only when subscreen
                    if( currentScreen->motherScreen != nullptr 
                    &&  currentScreen->motherScreen->subScreens.size() > 1) {
                        switchFocus();
                    }
                    break;
                case UP_KEY: case DOWN_KEY:
                    doMove(c == UP_KEY ? 1 : 0);
                    break;
                case CONFIRM_KEY:
                    switch(currentItem->getType()) {
                        case BasicItem::CHECKBOX:
                            cCast = (CheckBox*)currentItem;
                            cCast->setValue(*(cCast->getValue())?false:true);
                            if(*cCast->onCheckBoxChange != nullptr)
                                (*(cCast->onCheckBoxChange))();
                            cCast = nullptr;
                            break;
                        case BasicItem::BUTTON:
                            bCast = (Button*)currentItem;
                            if(*bCast->onButtonPress != nullptr) 
                                (*(bCast->onButtonPress))();
                            bCast = nullptr;
                            break;
                        #if CONFIRM_KEY == ' '
                        case BasicItem::TEXTBOX:
                            if (c == ' ')
                                goto textBoxHandler;
                            break;
                        #endif
                    }
                    if(currentItem->onDraw != nullptr) (*(currentItem->onDraw))();
                    currentItem->draw(true);
                    break;
                default:
                    textBoxHandler:
                        if(currentItem != nullptr) {
                            if(currentItem->getType() == BasicItem::TEXTBOX) {
                                tbCast = (TextBox*)currentItem;
                                if(c != KEY_BACKSPACE && !(*tbCast->filter)(c)) continue;
                                if(*tbCast->onKeyPress == nullptr) {
                                    tbCast->defaultKeyPressEvent(c);
                                }else{
                                    (*(tbCast->onKeyPress))(c);
                                }
                                tbCast = nullptr;

                            }
                        }
                        
                    break;
            }
            refresh();
        }
    }

    void Screen::update() {
        clear();
        if(subScreens.size() > 0) {
            for(auto &n : subScreens) {
                currentScreen = n;
                if(currentScreen->border) currentScreen->drawFrame(0);
                currentScreen->drawItems();
            }
            currentScreen = subScreens.at(this->subScreenIndex);
            currentScreen->drawFrame(currentScreen->isDragging ? 2 : 1); //drag frame
            currentItem = currentScreen->getFirstInteractableItem();
        }else{
            currentScreen = this;
            if(this->border) drawFrame(1);
            drawItems();
        }
        if(currentItem != nullptr) {
            if(currentItem->onDraw != nullptr) (*(currentItem->onDraw))();
                currentItem->draw(true);
        }
        if(currentScreen->onFocus != nullptr) (*(currentScreen->onFocus))();
        
        refresh();
        
    }

    void Screen::draw() {
        update();
        currentScreen->run(); 
    }

    void Screen::centerItem(Screen::axis pAxis, BasicItem *i) {
        switch(pAxis) {
            case X:
                switch(i->getType()) {
                    case BasicItem::TEXTBOX:
                        i->setX((width / 2) - ((i->name.length() /2) + 
                                ((((TextBox*) i)->maxLength + 4) / 2)));
                        break;
                    case BasicItem::CHECKBOX:
                        i->setX((width / 2) - ((i->name.length() /2) + 4));
                        break;
                    case BasicItem::BUTTON:
                        i->setX((width / 2) - ((i->name.length() /2) + 2));
                        break;
                    default:
                        i->setX((width / 2) - (i->name.length() /2));
                        break;
                }
                break;
            case Y:
                i->setY(height / 2);
                break;
            default:
                centerItem(X,i);
                centerItem(Y,i);
                break;
        }
    }
    void Screen::addSubScreen(Screen *i) {
        i->motherScreen = this;
        subScreens.push_back(i);
    }

    void Screen::setTitle(std::string s) {
        title = s;
    }
    void Screen::setBorder(bool v) {
        border = v;
    }
}