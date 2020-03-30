#ifndef CFILTER_H
#define CFILTER_H

class CFilter
{
  public:
    CFilter(float k, float startVal);

    template<typename T>
    T smooth(T value);
    void reset();
    void setInitVal(float val);

  private:
    float* mValue;
    float* mK;
    float initVal;
};


CFilter::CFilter(float k, float startVal = 0.0): initVal(startVal){
  mValue = new float;
  mK = new float;
  *mValue = initVal;
  *mK = k;
}

void CFilter::setInitVal(float val){
  *mValue = val;
}

template<typename T>
T CFilter::smooth(T value) {
  *mValue = (1.0 - *mK) * (*mValue) + *mK * value;
  return (T)*mValue;
}

void CFilter::reset(){
  *mValue = initVal;
}

#endif /* CFILTER_H */
