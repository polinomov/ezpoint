
//const myPics = document.getElementById('myPics');
//const context = myPics.getContext('2d');

var htmlCanvas;
var context;
class GlobVars {
    mouseX = 0;
    mouseY = 0;
}
var gv = new GlobVars();



var gCamRotCb;



function callWasm1() {
    console.log("CallWasm");
    Module['canvas'] = document.getElementById('myPics');
    const result = Module.ccall('CallCFunc', 'number');
}

function OnTest() {
    console.log("Hello TestMe JS-\n");
    test_cb = Module.cwrap('OnTestJS', 'number', ['number']);
    test_cb(4);
    //resize_cb = Module.cwrap('CallCFunc2', 'number', ['number','number'] , { async: true });
    //resize_cb(window.innerWidth, window.innerHeight);
    /*
    let promise = new Promise(function(resolve, reject) {
        console.log("--Exe promise---");
        setTimeout(() => {
            console.log("Delayed for 1 second.");
          }, "10000")
          
        console.log("--Exe promise 2" );
         resolve();
    });
 
    console.log("---here--------");
    
    promise.then(
        result => alert(result), 
        error => alert(error) 
    );
    */
    
}

//function OnDraw() {
//}

function OnMouseMove(e) {
    
    gv.mouseX = e.offsetX;
    gv.mouseY = e.offsetY;
    OnDraw();
}

function OnFileSelected(input) {
    //console.log("---fileSelected-- " + input.files[0].name);
    document.getElementById('GFG').innerHTML='Reading: ' + input.files[0].name;
    var file = input.files[0];
    if (!file) {
        console.log("NO FILE");
        return;
    }
    console.log("YES FILE size=" + input.files[0].size);
    var fz = input.files[0].size;
    var reader = new FileReader();
    reader.onload = function (e) {
        // binary data
       
       console.log("-reader-onload-");
       var data = reader.result;
       var array = new Uint8Array(data);
       var res_ptr = Module._malloc(fz);
       Module.HEAPU8.set(array, res_ptr);
       file_cb = Module.cwrap('FileBinData', 'number', ['arrayPointer', 'number'],{ async: true });
       file_cb(res_ptr, fz);
      // Module._free(res_ptr);
    };
    reader.onloadend = function (e) {
       console.log("load done");
    }

    reader.onerror = function (e) {
        // error occurred
        console.log('Error : ' + e.type);
    };
   // document.getElementById('GFG1').innerHTML='-JSStart'
    reader.readAsArrayBuffer(input.files[0]);  
   // document.getElementById('GFG1').innerHTML='-JSDone'
}

function OnFileOpen() {
    document.getElementById('attachment').click();
}


function resizeCanvas() {
    // console.log("Resize w=" + window.innerWidth + " h=" + window.innerHeight);
    resize_cb = Module.cwrap('CallCFunc', 'number', ['number','number']);
    resize_cb(window.innerWidth, window.innerHeight);
}


class ProscessEventsClass{
    constructor() {
        this.ctrlOn = 0;
        this.mouseOn = 0;
        this.posX = -1;
        this.posY = -1;
        //console.log("ctor");
    };

    onMouseUp(e){
        this.mouseOn = 0;
        this.posX = -1;
        this.posY = -1;
        //console.log(e);
    }

    onMouseClick(e){
        //console.log(e);
    }

    onMouseDown(e){
        this.mouseOn = 1;
        this.posX = e.clientX;
        this.posY = e.clientY;
        //console.log(e);
    }

    onMouseMove(e){
        if(this.mouseOn==1){
            let dx = e.clientX - this.posX;
            let dy = e.clientY - this.posY;
            let adx = (dx>0) ? dx:-dx;
            let ady = (dy>0) ? dy:-dy;
            if(adx > ady){
                let vx = adx *10000/window.innerWidth;
                if( dx > 0){
                    gCamRotCb(1, 0, 0,vx);
                }
                if(dx<0){
                    gCamRotCb(-1, 0, 0,vx);
                }
            }
            else{
                let vy = ady *10000/window.innerHeight;
                if( dy > 0){
                    gCamRotCb(0, -1, 0,vy);
                }
                if(dy<0){
                    gCamRotCb(0, 1, 0,vy);
                }
            }
            this.posX = e.clientX;
            this.posY = e.clientY;
        }
    }

    onMouseWheel(e){
       // console.log("x=" + e.deltaX + " y="+ e.DeltaY);
    } 

    onWheel(e){
        if(e.deltaY > 0){
            gCamRotCb( 0, 1, 1,0);
        }else{
            gCamRotCb( 0, -1, 1,0); 
        }
    } 

    onKeyUp(e){
        if(e.key=="Control"){
            this.ctrlOn = 0;
        }
    }

    onKeyDown(e){
         switch(e.key){
            case "ArrowUp":    gCamRotCb( 0, 1, this.ctrlOn,200); break;
            case "ArrowDown":  gCamRotCb( 0,-1, this.ctrlOn,200); break;
            case "ArrowLeft":  gCamRotCb(-1, 0, this.ctrlOn,200); break;
            case "ArrowRight": gCamRotCb( 1, 0, this.ctrlOn,200); break;
            case "Control":    this.ctrlOn = 1; break;
        } 
    }
}

var ProcessEvents = new ProscessEventsClass();

function onTestCheckClick(cb){
    ch_cb = Module.cwrap('OnDebugCheckBox', 'number', ['number']);
    if(cb.checked){
         ch_cb(1);
    }else{
        ch_cb(0);
    }
 }

function OnTestButton(){

}

function OnLoaded() {  
    // init canvas
    htmlCanvas = document.getElementById('myPics');
    context = htmlCanvas.getContext('2d');
    resizeCanvas();
 }

function OnStart()
{
    console.log("-OnStart-\n");
    gCamRotCb = Module.cwrap('CameraRotateJS', 'number','number','number','number' ['number']);
    resizeCanvas();
    window.addEventListener('resize',  resizeCanvas, false);
    window.addEventListener('keydown',   (event) => {ProcessEvents.onKeyDown(event);}, false);
    window.addEventListener('keyup',     (event) => {ProcessEvents.onKeyUp(event);}, false);
    const el = document.getElementById("canvas");
    el.addEventListener('click',     (event) => {ProcessEvents.onMouseClick(event);}, false);
    el.addEventListener('mouseup',   (event) => {ProcessEvents.onMouseUp(event);}, false);
    el.addEventListener('mousemove', (event) => {ProcessEvents.onMouseMove(event);}, false);
    el.addEventListener('mousedown', (event) => {ProcessEvents.onMouseDown(event);}, false);
    el.addEventListener('mousewheel',(event) => {ProcessEvents.onMouseWheel(event);}, false);
    el.addEventListener('wheel',     (event) => {ProcessEvents.onWheel(event);}, false);
}



//OnStart();