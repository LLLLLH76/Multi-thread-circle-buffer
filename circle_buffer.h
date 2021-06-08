enum states{EMPTY, NORMAL, FULL};

struct bf_element{
	int content;
	bool is_element;
};

class CircleBuffer{
    //FIFO
    public:
        int size;
        bf_element* buffer;
        int start;
        int end;
        states state;
        void set(int k);
        int get();
        void put(int x);
};

void CircleBuffer::set(int k){
    size = k;
    start = 0;
    end = 0;
    state = EMPTY;
    buffer = (bf_element*)malloc(size*sizeof(bf_element));
    for(int i = 0; i<size; i++){
        buffer[i].content = 0;
        buffer[i].is_element = false;
    }
    return;
}

int CircleBuffer::get(){
	int res = buffer[start].content;
    if(start == end){
        switch (state){
            case EMPTY:
                //cout<<"\nbuffer is empty.";
                return -1;
                break;
            case NORMAL:
                state = EMPTY;
                buffer[end].is_element = false;
                return res;
                break;
            case FULL:
            	state = NORMAL;
            	buffer[end].is_element = false;
                start = (start+1)%size;
                end = (end+size-1)%size;
                return res;
                break;
            default:
                break;
        }
    }
    else{
        buffer[start].is_element = false;
        start = (start+1)%size;
        state = NORMAL;
        return res;
    }
}

void CircleBuffer::put(int x){
    if(start == end && state == EMPTY){
        buffer[end].content = x;
        buffer[end].is_element = true;
        state = NORMAL;
        return;
    }
    end = (end+1)%size;
    buffer[end].content = x;
    buffer[end].is_element = true;
    if(state == FULL)
    	start = (start+1)%size;
    if((end+1)%size == start && buffer[(start+size-1)%size].is_element == true){
        state = FULL;
        //cout<<"buffer is full.\n";
    }
    return;
}