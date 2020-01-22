//
// Created by Administrator on 2020/1/20 0020.
//
#include <stdint.h>
class  AudioData {
private:
    uint8_t * a;
    int  size;
public:
    AudioData(){


    }
    ~AudioData(){


    }
    uint8_t *  getData(){

        return   this->a;

    }
    int  getSize(){
        return   this->size;

    }
    void    setData( uint8_t *  data){

        this->a   = data;

    }
    void    setSize(int  size){
        this->size = size;

    }
};


