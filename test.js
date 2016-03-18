var distance = require('./build/Release/distance');

distance.open("/home/max/word2vec/google/vectors_text8.bin",function(err,data){
	if(!err) {

		console.log(distance.compare("yes","answers"));
		console.log(distance.compare("yes","ja"));
		console.log(distance.compare("yes","linklater"));
		console.log(distance.compare("yes","miya"));
		console.log(distance.compare("yes","nakatomi"));

		return;

		var words = ["hello","world","it","is","a","nice","day","wefihadlkjnasdlkjasvjhdfkjhsdf"];
		for(var i=0;i<words.length;i++) {
			console.log(distance.get(words[i]));
		}
	}

});