var distance = require('./build/Release/distance');

distance.open("/Users/max/content/word2vec/hnphrase0.bin",function(err,data){
	if(!err) {
		var words = ["hello","world","it","is","a","nice","day","wefihadlkjnasdlkjasvjhdfkjhsdf"];
		for(var i=0;i<words.length;i++) {
			console.log(distance.get(words[i]));
		}

	}
});