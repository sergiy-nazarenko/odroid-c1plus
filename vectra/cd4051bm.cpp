#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <wiringPi.h>


class CD4051BM {
public:
    int _pin_a, _pin_b, _pin_c;
    explicit CD4051BM(): _pin_a(0), _pin_b(1), _pin_c(2)  {}
    explicit CD4051BM(int a, int b, int c):
             _pin_a(a), _pin_b(b), _pin_c(c) 
    {
        wiringPiSetup () ;
        pinMode (_pin_a, OUTPUT) ;
        pinMode (_pin_b, OUTPUT) ;
        pinMode (_pin_c, OUTPUT) ;
    }
    bool setState(int s)
    {
        switch(s)
        {
            case 0:
                digitalWrite (_pin_a, LOW) ;
                digitalWrite (_pin_b, LOW) ;
                digitalWrite (_pin_c, LOW) ;
                break;
            case 1:
                digitalWrite (_pin_a, HIGH) ;
                digitalWrite (_pin_b, LOW) ;
                digitalWrite (_pin_c, LOW) ;
                break;
            case 2:
                digitalWrite (_pin_a, LOW) ;
                digitalWrite (_pin_b, HIGH) ;
                digitalWrite (_pin_c, LOW) ;
                break;
            case 3:
                digitalWrite (_pin_a, HIGH) ;
                digitalWrite (_pin_b, HIGH) ;
                digitalWrite (_pin_c, LOW) ;
                break;
            case 4:
                digitalWrite (_pin_a, LOW) ;
                digitalWrite (_pin_b, LOW) ;
                digitalWrite (_pin_c, HIGH) ;
                break;
            case 5:
                digitalWrite (_pin_a, HIGH) ;
                digitalWrite (_pin_b, LOW) ;
                digitalWrite (_pin_c, HIGH) ;
                break;
            case 6:
                digitalWrite (_pin_a, LOW) ;
                digitalWrite (_pin_b, HIGH) ;
                digitalWrite (_pin_c, HIGH) ;
                break;
            case 7:
                digitalWrite (_pin_a, HIGH) ;
                digitalWrite (_pin_b, HIGH) ;
                digitalWrite (_pin_c, HIGH) ;
                break;
            default:
                return false;
        }
        return true;
    }
};

namespace py = pybind11;

PYBIND11_MODULE(cd4051bm, m) 
{
    m.doc() = "binding CD4051BM"; 
    py::class_<CD4051BM, std::shared_ptr<CD4051BM> > cd40(m, "CD4051BM");
    cd40.def(py::init<>());
    cd40.def(py::init<int, int, int>(), "def constructor");
    cd40.def("set", &CD4051BM::setState);
}
