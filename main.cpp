#include <SDL2/SDL.h>
#include <cmath>
#include <utility>


const size_t TEST_DIST = 400 /// constant for minimal available instance
int TEST_STEPS = 512; /// constant of maximum number of steps

uint8_t* red = NULL; /// constant for defining the array for Red segment of RGB colors
uint8_t* green = NULL; /// constant for defining the array for Green segment of RGB
colors
uint8_t* blue = NULL; /// constant for defining the array for Blue segment of RGB
colors

#define MIN_X -2.1
#define MAX_X 0.67
#define MIN_Y (-MAX_Y)
#define MAX_Y (((MAX_X - MIN_X) * ((double)HEIGHT / WIDTH)) / 2)

#define MOVE_PRECISION 10

#define WIDTH 1000
#define HEIGHT 600

SDL_Window* gWindow; /// SDL2 Window
SDL_Renderer* gRenderer; /// SDL2 Renderer
SDL_Texture* gScreen; /// SDL2 texture for Screen
SDL_Texture* gTemp; /// SDL2 Temporary Texture

/// Class that defines a double part of the Mandelbrot fractal

class DoubleSelection{
    public:
    DoubleSelection(DoubleSelection& other) = default; /// copy constructor
    DoubleSelection() = default; /// default constructor
    DoubleSelection(const double minX,
                    const double minY,
                    const double maxX,
                    const double maxY): minPoint(std::make_pair(minX, minY)),
                                        maxPoint(std::make_pair(maxX, maxY)) { } /// constructor of minimal and maximum point of the selection
    
    ~DoubleSelection()=default; /// default destructor

//! += operator with another point
/*!
+= operator for another point represented by min and max value

@param val value of mininum and maximum coordinates of the point
*/
    DoubleSelection& operator += (const std::pair<std::pair<double, double>,
                                                std::pair<double, double> > &value){
        minPoint.first += value.first.first;
        minPoint.second += value.first.second;

        maxPoint.first += value.second.first;
        maxPoint.second += value.second.second;

        return *this;
    }


//! -= operator with another point
/*!
-= operator for another point represented by min and max value

@param val value of mininum and maximum coordinates of the point
*/
    DoubleSelection& operator -= (const std::pair<std::pair<double, double>,
                                                std::pair<double, double> > &value){
        minPoint.first -= value.first.first;
        minPoint.second -= value.first.second;

        maxPoint.first -= value.second.first;
        maxPoint.second -= value.second.second;

        return *this;
    }

    //getters
    double getMinX() const{
        return minPoint.first;
    }
    double getMinY() const{
        return minPoint.second;
    }

    double getMaxX() const{
        return maxPoint.first;
    }
    double getMaxY() const{
        return maxPoint.second;
    }

    //setters
    void setMinX(const double &value){
        minPoint.first = value;
    }
    void setMinY(const double &value){
        minPoint.second = value;
    }

    void setMaxX(const double &value){
        maxPoint.first = value;
    }
    void setMaxY(const double &value){
        maxPoint.second = value;
    }
    private:
        //x, y
        std::pair<double, double> minPoint;
        std::pair<double, double> maxPoint;
};

/// Selection of integers within a Mandelbrot fractal
class IntSelection{
    public:
        IntSelection(IntSelection& other) = default;
        IntSelection()=default;
        IntSelection(const int minX,
                    const int minY,
                    const int maxX,
                    const int maxY): minPoint(std::make_pair(minX, minY)),
                                        maxPoint(std::make_pair(maxX, maxY)) { }
        ~IntSelection()=default;
        //getters
        int getMinX() const{
            return minPoint.first;
        }
        int getMinY() const{
            return minPoint.second;
        }
        int getMaxX() const{
            return maxPoint.first;
        }
        int getMaxY() const{
            return maxPoint.second;
        }

        //setters
        void setMinX(const int value){
            minPoint.first = value;
        }
        void setMinY(const int value){
            minPoint.second = value;
        }
        void setMaxX(const int value){
            maxPoint.first = value;
        }
        void setMaxY(const int value){
            maxPoint.second = value;
        }

    private:
        std::pair<int, int> minPoint;
        std::pair<int, int> maxPoint;
};


/// Class that represents a complex number
class Complex{
    public:
        Complex(double _real=0, double _imaginary=0):real(_real), imaginary(_imaginary) {}

        Complex& operator += (const Complex &other){
            real += other.real;
            imaginary += other.imaginary;
            return *this;
        }

        double distance() {
            return real * real + imaginary * imaginary;
        }
        //getters
        double getReal() const{
            return real;
        }
        double getImaginary() const{
            return imaginary;
        }
        //setters
        void setReal(const double value) {
            real = value;
        }
        void setImaginary(const double value){
            imaginary = value;
        }
    private:
        double real;
        double imaginary;
};

enum Response { RESP_QUIT, RESP_UP, RESP_DOWN, RESP_LEFT, RESP_RIGHT, RESP_ZOOM_IN, 
                RESP_ZOOM_OUT, RESP_RESET, RESP_NONE, RESP_EVOLVE, RESP_DEGENERATE,
                RESP_JUMP_UP, RESP_JUMP_DOWN };

void init_colors() {
    free(red);
    free(green);
    free(blue);
    red = (uint8_t*)malloc(TEST_STEPS);
    green = (uint8_t*)malloc(TEST_STEPS);
    blue = (uint8_t*)malloc(TEST_STEPS);
    for(size_t i = 0; i < TEST_STEPS; i++) {
        double angle = M_PI * 2 / TEST_STEPS * i + 3.7;
        red[i] = sin(M_PI_2 * (sin(angle) + 1) / 2) * 0xFF;
        green[i] = sin(M_PI_2 * (sin(angle + M_PI_2) + 1) / 2) * 0xFF;
        blue[i] = sin(M_PI_2 * (sin(angle + M_PI) + 1) / 2) * 0xFF;
    }
}

void init() {
    SDL_Init(SDL_INIT_VIDEO);
    gWindow = SDL_CreateWindow("Mandelbrot", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
#ifdef FULLSCREEN
    SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_FULLSCREEN);
#endif
    gRenderer= SDL_CreateRenderer(gWindow, -1, 0);
    gScreen = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 
        WIDTH, HEIGHT);
    gTemp = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 
        WIDTH, HEIGHT);
    init_colors();
}

void quit() {
    SDL_DestroyTexture(gScreen);
    SDL_DestroyTexture(gTemp);
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    SDL_Quit();
}

void square(const Complex &comp, Complex &res) {
    res.setReal((comp.getReal() - comp.getImaginary()) * (comp.getReal() + comp.getImaginary()));
    res.setImaginary(comp.getReal() * comp.getImaginary());
    res += Complex(0, res.getImaginary());
}

void add(Complex &res, const Complex &added) {
    res += added;
}

int count_steps(const Complex &comp) {
    size_t res = 0;
    Complex temp1, temp2;
    do {
        square(temp1, temp2);
        temp2 += comp;
        temp1 = temp2;
        res++;
    } while(res < TEST_STEPS && temp2.distance() < TEST_DIST);
    return res;
}

void present() {
    SDL_SetRenderTarget(gRenderer, gScreen);
    SDL_RenderCopy(gRenderer, gTemp, NULL, NULL);
    SDL_SetRenderTarget(gRenderer, NULL);
    SDL_RenderCopy(gRenderer, gTemp, NULL, NULL);
    SDL_RenderPresent(gRenderer);
    SDL_SetRenderTarget(gRenderer, gTemp);
}

void draw(const DoubleSelection &ds, const IntSelection &is, const bool pres) {
    int width = is.getMaxX() - is.getMinX();
    int height = is.getMaxY() - is.getMinY();
    double hUnit = (ds.getMaxX() - ds.getMinX()) / (WIDTH / (is.getMaxX() - is.getMinX())) / width;
    double vUnit = (ds.getMaxY() - ds.getMinY()) / (HEIGHT / (is.getMaxY() - is.getMinY())) / height;
    Complex comp;
    for(unsigned i = is.getMinY(); i < is.getMaxY(); i++) {
        comp.setImaginary(ds.getMaxY() - i * vUnit);
        for(unsigned j = is.getMinX(); j < is.getMaxX(); j++) {
            comp.setReal(ds.getMinX() + j * hUnit);
            size_t steps = count_steps(comp);
            if(steps != TEST_STEPS)
                SDL_SetRenderDrawColor(gRenderer, red[steps], green[steps], blue[steps], 0xFF);
            else
                SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);
            SDL_RenderDrawPoint(gRenderer, j, i);
            
        }
        if(pres) 
            present();
    }
}

enum Response handle_input() {
    SDL_Event e;
    SDL_WaitEvent(&e);
    static bool input = true;
    switch(e.type) {
    case SDL_QUIT: return RESP_QUIT;
    case SDL_KEYDOWN:
        if(input) {
            input = false;
            switch(e.key.keysym.sym) {
                case SDLK_UP: return RESP_UP;
                case SDLK_DOWN: return RESP_DOWN;
                case SDLK_LEFT: return RESP_LEFT;
                case SDLK_RIGHT: return RESP_RIGHT;
                case SDLK_PAGEUP:
                case SDLK_z:
                    return RESP_ZOOM_IN;
                case SDLK_PAGEDOWN:
                case SDLK_x:
                    return RESP_ZOOM_OUT;
                case SDLK_SPACE: return RESP_RESET;
                case SDLK_q: return RESP_DEGENERATE;
                case SDLK_w: return RESP_EVOLVE;
                case SDLK_a: return RESP_JUMP_DOWN;
                case SDLK_s: return RESP_JUMP_UP;
                case SDLK_ESCAPE:
                    return RESP_QUIT;
                default: return RESP_NONE;
            }
        }
        break;
        case SDL_KEYUP:
            input = true;
            return RESP_NONE;
    }
    return RESP_NONE;
}

void move_up(DoubleSelection &ds, IntSelection &is, double dStep, int iStep) {
    SDL_SetRenderTarget(gRenderer, gTemp);
    SDL_Rect dst = {0, iStep, WIDTH, HEIGHT};
    SDL_RenderCopy(gRenderer, gScreen, NULL, &dst);

    ds += std::make_pair(std::make_pair(0, dStep), std::make_pair(0, dStep));
    IntSelection other(0, 0, WIDTH, iStep);
    is = other;
    draw(ds, is, false);
}

void move_down(DoubleSelection &ds, IntSelection &is, double dStep, int iStep) {
    SDL_SetRenderTarget(gRenderer, gTemp);
    SDL_Rect dst = {0, -iStep, WIDTH, HEIGHT};
    SDL_RenderCopy(gRenderer, gScreen, NULL, &dst);

    ds -= std::make_pair(std::make_pair(0, dStep), std::make_pair(0, dStep));
    IntSelection other(0, HEIGHT - iStep, WIDTH, HEIGHT);
    is = other;
    draw(ds, is, false);
}

void move_left(DoubleSelection &ds, IntSelection &is, double dStep, int iStep) {
    SDL_SetRenderTarget(gRenderer, gTemp);
    SDL_Rect dst = {iStep, 0, WIDTH, HEIGHT};
    SDL_RenderCopy(gRenderer, gScreen, NULL, &dst);

    ds -= std::make_pair(std::make_pair(dStep, 0.0), std::make_pair(dStep, 0.0));
    IntSelection other(0, HEIGHT - iStep, WIDTH, HEIGHT);
    is = other;
    draw(ds, is, false);
}

void move_right(DoubleSelection &ds, IntSelection &is, double dStep, int iStep) {
    SDL_SetRenderTarget(gRenderer, gTemp);
    SDL_Rect dst = {-iStep, 0, WIDTH, HEIGHT};
    SDL_RenderCopy(gRenderer, gScreen, NULL, &dst);
    ds += std::make_pair(std::make_pair(dStep, 0.0), std::make_pair(dStep, 0.0));
    IntSelection other(WIDTH - iStep, 0, WIDTH, HEIGHT);
    is = other;
    draw(ds, is, false);
}

void zoom_in(DoubleSelection &ds, IntSelection &is, double* dhStep, double* dvStep,
        int* ihStep, int* ivStep) {
    SDL_SetRenderTarget(gRenderer, gTemp);
    ds += std::make_pair(std::make_pair(*dhStep, *dvStep), std::make_pair(0.0, 0.0));
    ds -= std::make_pair(std::make_pair(0.0, 0.0), std::make_pair(*dhStep, *dvStep));
    IntSelection other(0, 0, WIDTH, HEIGHT);
    is = other;
    draw(ds, is, true);
    *dhStep = (ds.getMaxX() - ds.getMinX()) / MOVE_PRECISION;
    *dvStep = (ds.getMaxY() - ds.getMinY()) / MOVE_PRECISION;
    *ihStep = WIDTH / MOVE_PRECISION;
    *ivStep = HEIGHT / MOVE_PRECISION;
}

void zoom_out(DoubleSelection &ds, IntSelection &is, double* dhStep, double* dvStep,
        int* ihStep, int* ivStep) {
    SDL_SetRenderTarget(gRenderer, gTemp);
    ds -= std::make_pair(std::make_pair(*dhStep, *dvStep), std::make_pair(0.0, 0.0));
    ds += std::make_pair(std::make_pair(0.0, 0.0), std::make_pair(*dhStep, *dvStep));
    IntSelection other(0, 0, WIDTH, HEIGHT);
    is = other;
    draw(ds, is, true);
    *dhStep = (ds.getMaxX() - ds.getMinX()) / MOVE_PRECISION;
    *dvStep = (ds.getMaxY() - ds.getMinY()) / MOVE_PRECISION;
    *ihStep = WIDTH / MOVE_PRECISION;
    *ivStep = HEIGHT / MOVE_PRECISION;
}

void redraw(DoubleSelection &ds, IntSelection *is) {
    SDL_SetRenderTarget(gRenderer, gTemp);
    if(!is) {
        IntSelection is1(0, 0, WIDTH, HEIGHT);
        draw(ds, is1, true);
    } else
        draw(ds, *is, true);
    present();
}

void reset(DoubleSelection &ds, IntSelection &is, double* dhStep, double* dvStep,
        int* ihStep, int* ivStep) {
    DoubleSelection other(MIN_X, MIN_Y, MAX_X, MAX_Y);
    ds = other;
    IntSelection other_int(0, 0, WIDTH, HEIGHT);
    is = other_int;
    SDL_SetRenderTarget(gRenderer, gTemp);
    draw(ds, is, false);
    present();
    *dhStep = (ds.getMaxX() - ds.getMinX()) / MOVE_PRECISION;
    *dvStep = (ds.getMaxY() - ds.getMinY()) / MOVE_PRECISION;
    *ihStep = WIDTH / MOVE_PRECISION;
    *ivStep = HEIGHT / MOVE_PRECISION;
}

void loop(DoubleSelection &ds, IntSelection &is, double* dhStep, double* dvStep, 
        int* ihStep, int* ivStep) {
    while(true) {
        switch(handle_input()) {
            case RESP_UP: 
                move_up(ds, is, *dvStep, *ivStep);
                break;
            case RESP_DOWN:
                move_down(ds, is, *dvStep, *ivStep);
                break;
            case RESP_LEFT:
                move_left(ds, is, *dhStep, *ihStep);
                break;
            case RESP_RIGHT:
                move_right(ds, is, *dhStep, *ihStep);
                break;
            case RESP_ZOOM_IN:
                zoom_in(ds, is, dhStep, dvStep, ihStep, ivStep);
                break;
            case RESP_ZOOM_OUT:
                zoom_out(ds, is, dhStep, dvStep, ihStep, ivStep);
                break;
            case RESP_JUMP_UP:
                TEST_STEPS *= 2;
                init_colors();
                redraw(ds, nullptr);
                break;
            case RESP_JUMP_DOWN:
                if(TEST_STEPS) 
                    TEST_STEPS /= 2;
                init_colors();
                redraw(ds, nullptr);
                break;
            case RESP_EVOLVE:
                TEST_STEPS++;
                init_colors();
                redraw(ds, nullptr);
                break;
            case RESP_DEGENERATE:
                if(TEST_STEPS) 
                    TEST_STEPS--;
                init_colors();
                redraw(ds, nullptr);
                break;
            case RESP_RESET:
                reset(ds, is, dhStep, dvStep, ihStep, ivStep);
                break;
            case RESP_QUIT: return;
            case RESP_NONE: break;
        }
        present();
    }
}

void proceed() {
    DoubleSelection ds;
    IntSelection is;
    double dhStep, dvStep;
    int ihStep, ivStep;
    reset(ds, is, &dhStep, &dvStep, &ihStep, &ivStep);
    loop(ds, is, &dhStep, &dvStep, &ihStep, &ivStep);
}

int main(int argc, char *argv[]) {
    init();
    proceed();
    quit();
    return 0;
}
