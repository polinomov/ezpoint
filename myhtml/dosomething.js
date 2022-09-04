
/*
self.Module = {
    locateFile: function (s) {
        console.log(s);
        return s;
    }
    // Add this function
    onRuntimeInitialized: function() {
        test();
    }
};
*/

Module['onRuntimeInitialized'] = function() {
       console.log("wasm loaded ");
       var x=Module.ccall("doubleIt","number",["number"],[20]);
       alert(x);
    }

self.importScripts("js_plumbing.js"); 

self.test = function() {
	console.log("we may safely use self.data and self.Module now");
    // we may safely use self.data and self.Module now!
   // console.log(self.Module.ccall("test")); // works!
}

// Message from html
onmessage = (event) => {
  const { data } = event;
  const primes = doWork(parseInt(data));
};



function doWork(num) 
{
	console.log("----importScripts=== " + num);
	if (Module['onRuntimeInitialized']) Module['onRuntimeInitialized']();
	if(num == 12345)
	{
		//importScripts('js_plumbing.js');
		postMessage(777);
	}
	if(num==777)
	{
		const result = Module.ccall('Add','number');		
	}
	//importScripts("js_plumbing.ww.js");
	   //importScripts('js_plum);
  //if(i < 100) {
	    
		// const result = Module.ccall('Add','number');

		//i = si + 1;
		//postMessage(i);
		//postMessage(i+1);
		//postMessage(i);
		//postMessage(i);
		//postMessage(i);
  //}
  return num;
}

//doWork();