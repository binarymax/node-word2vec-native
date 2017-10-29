# node-word2vec-native
Port of the word2vec distance program to a native node.js addon

## Installation

Install it from npm

```npm install word2vec-native```

## Usage

This addon does not train vectors.  You may use this to load an existing binary word2vec model file from disk.  Once the model is loaded, you may perform the following two operations: get and compare.

```javascript
var distance = require('word2vec-native');

distance.open('my-word2vec-model.bin',function(err){
    if(!err) {
        console.log(distance.compare("foo","bar"));
        console.log(distance.get("foo"));
    }
});
```

### Get
Given a single term, returns a list of the most similar terms.

### Compare
Given two terms, returns the cosine distance between their respective vectors.

See the test.js code for an example

## Credit
Credit goes to Tomas Mikolov for his original word2vec implementation. Original code from https://code.google.com/p/word2vec/
