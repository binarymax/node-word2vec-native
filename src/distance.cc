// distance.cc
#include <node.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <iostream>

namespace word2vec {

using std::string;
using v8::Exception;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Array;
using v8::Number;
using v8::Object;
using v8::String;
using v8::Value;
using v8::Null;

const long long max_size = 2000;         // max length of strings
const long long closest = 40;            // number of closest words that will be shown
const long long max_w = 50;              // max length of vocabulary entries

char *bestw[closest];
char file_name[max_size];
float len;
long long words, size;
float *matrix;
char *vocab;


/*
@param1 -> Filename for vector binary
@param2 -> Callback for return
*/
void Open(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
	FILE *file;
	long long a, b;

	if (args.Length() != 2) {
    	isolate->ThrowException(Exception::TypeError(
        	String::NewFromUtf8(isolate, "Two arguments are required for distance.open(filename,callback)")));
    	return;
	}

	if (!args[0]->IsString()) {
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "First argument must be a String")));
		return;
	}

	Local<Function> cb = Local<Function>::Cast(args[1]);

	//Open the file, using param1
	v8::String::Utf8Value param1(args[0]->ToString());
	string filename = string(*param1);
	file = fopen(filename.c_str(), "rb");

	if (file == NULL) {
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "Input File Not Found!")));
		return;
	}

	fscanf(file, "%lld", &words);
	fscanf(file, "%lld", &size);


	//Read the file into the datastructure
	vocab = (char *)malloc((long long)words * max_w * sizeof(char));
	for (a = 0; a < closest; a++) {
		bestw[a] = (char *)malloc(max_size * sizeof(char));
	}
	matrix = (float *)malloc((long long)words * (long long)size * sizeof(float));
	if (matrix == NULL) {
		//printf("Cannot allocate memory: %lld MB    %lld  %lld\n", (long long)words * size * sizeof(float) / 1048576, words, size);
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "Cannot allocate memory!")));
		return;
	}
	for(b = 0; b < words; b++) {
		a = 0;
		while (1) {
			vocab[b * max_w + a] = fgetc(file);
			if (feof(file) || (vocab[b * max_w + a] == ' ')) break;
			if ((a < max_w) && (vocab[b * max_w + a] != '\n')) a++;
		}
		vocab[b * max_w + a] = 0;
		for (a = 0; a < size; a++) {
			fread(&matrix[a + b * size], sizeof(float), 1, file);
		}
		len = 0;
		for (a = 0; a < size; a++) {
			len += matrix[a + b * size] * matrix[a + b * size];
		}
		len = sqrt(len);
		for (a = 0; a < size; a++) { 
			matrix[a + b * size] /= len;
		}
	}

	//Cleanup
	fclose(file);

	//Return data about the file
	const unsigned argc = 2;
	Local<Object> obj = Object::New(isolate);
	obj->Set(String::NewFromUtf8(isolate, "words"), Number::New(isolate, words));
	obj->Set(String::NewFromUtf8(isolate, "size"), Number::New(isolate, size));
	Local<Value> argv[argc] = { Null(isolate), obj };


	//Return
	cb->Call(Null(isolate), argc, argv);
	
}


void Get(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	char st1[max_size];
	char *bestw[closest];
	char st[100][max_size];
	float dist, bestd[closest], vec[max_size];
	long long a, b, c, d, cn, bi[100];

	if (args.Length() != 1) {
    	isolate->ThrowException(Exception::TypeError(
        	String::NewFromUtf8(isolate, "One argument is required for distance.get(word)")));
    	return;
	}

	if (!args[0]->IsString()) {
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "First argument must be a string")));
		return;
	}

	for (a = 0; a < closest; a++) bestw[a] = (char *)malloc(max_size * sizeof(char));

	//Init the lists
    for (a = 0; a < closest; a++) bestd[a] = 0;
    for (a = 0; a < closest; a++) bestw[a][0] = 0;

    //Cast the lookup word
    v8::String::Utf8Value param1(args[0]->ToString());
	string word1 = string(*param1);
	std::strcpy(st1, word1.c_str());


	Local<Object> data = Object::New(isolate);
	Local<Object> data_meta = Object::New(isolate);
	data_meta->Set(String::NewFromUtf8(isolate, "words"), Number::New(isolate, words));
	data_meta->Set(String::NewFromUtf8(isolate, "size"), Number::New(isolate, size));
	data->Set(String::NewFromUtf8(isolate, "meta"), data_meta);
	Local<Array> closest_words = Array::New(isolate);

	//Find the word in the vocabulary
	cn = 0;
	b = 0;
	c = 0;
	while (1) {
		st[cn][b] = st1[c];
		b++;
		c++;
		st[cn][b] = 0;
		if (st1[c] == 0) break;
		if (st1[c] == ' ') {
			cn++;
			b = 0;
			c++;
		}
	}
	cn++;
	for (a = 0; a < cn; a++) {
		for (b = 0; b < words; b++) {
			if (!strcmp(&vocab[b * max_w], st[a])) break;
		}
		if (b == words) b = -1;
		bi[a] = b;
		if (b == -1) {			
			//Word not found in vocabulary
			data->Set(String::NewFromUtf8(isolate,"words"),closest_words);
			args.GetReturnValue().Set(data);
			return;
		}
	}

	//Navigate vectors to get the list of closest words
	for (a = 0; a < size; a++) {
		vec[a] = 0;
	}
	for (b = 0; b < cn; b++) {
		if (bi[b] == -1) continue;
		for (a = 0; a < size; a++) {
			vec[a] += matrix[a + bi[b] * size];
		}
	}
	len = 0;
	for (a = 0; a < size; a++) {
		len += vec[a] * vec[a];
	}
	len = sqrt(len);
	for (a = 0; a < size; a++) {
		vec[a] /= len;
	}
	for (a = 0; a < closest; a++) {
		bestd[a] = -1;
	}
	for (a = 0; a < closest; a++) {
		bestw[a][0] = 0;
	}
	for (c = 0; c < words; c++) {
		a = 0;
		for (b = 0; b < cn; b++) {
			if (bi[b] == c) a = 1;
		}
		if (a == 1) continue;
		dist = 0;
		for (a = 0; a < size; a++) {
			dist += vec[a] * matrix[a + c * size];
		}
		for (a = 0; a < closest; a++) {
			if (dist > bestd[a]) {
				for (d = closest - 1; d > a; d--) {
					bestd[d] = bestd[d - 1];
					strcpy(bestw[d], bestw[d - 1]);
				}
				bestd[a] = dist;
				strcpy(bestw[a], &vocab[c * max_w]);
				break;
			}
		}
	}

	//Add the closest words to the return data
	for (a = 0; a < closest; a++) {
		Local<Object> close = Object::New(isolate);
		close->Set(String::NewFromUtf8(isolate,"word"),String::NewFromUtf8(isolate,bestw[a]));
		close->Set(String::NewFromUtf8(isolate,"dist"),Number::New(isolate,bestd[a]));
		closest_words->Set(a,close);
	}
	data->Set(String::NewFromUtf8(isolate,"words"),closest_words);

	//Return
	args.GetReturnValue().Set(data);

}

void init(Local<Object> exports) {
	NODE_SET_METHOD(exports, "open", Open);
	NODE_SET_METHOD(exports, "get", Get);
}

NODE_MODULE(distance, init)

}  // namespace word2vec