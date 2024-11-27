
  

var htmlCanvas;
var context;
class GlobVars {
    mouseX = 0;
    mouseY = 0;
}
var gv = new GlobVars();

var gCamRotCb;
var gCamMoveCb;
var gCamDbClickCB;
var gUIChangeCB;
var gMouseMoveCB;
var gMouseClickCB;
var gIdChanged = "unknown"
var gDisableRdb = 0;

function callWasm1() {
    console.log("CallWasm");
    Module['canvas'] = document.getElementById('myPics');
    const result = Module.ccall('CallCFunc', 'number');
}

function OnTest( v) {
    test_cb = Module.cwrap('OnTestJS', 'number', ['number']);
    test_cb(v);
}

function OnMouseMove(e) {
    gv.mouseX = e.offsetX;
    gv.mouseY = e.offsetY;
    OnDraw();
}


async function PostProcess() {
    return new Promise(() => {
        console.log("ISIDE PROMISE");
    });
}

async function OnLoadLas(input) {
    var fileNdx_ = 0;
    var numFiles_ = input.files.length;
    document.getElementById('GFG').innerHTML = 'Reading ... ' + numFiles_;
    console.log("OnLoadLas");
    var file = input.files[fileNdx_];
    if (!file) {
        console.log("NO FILE");
        return;
    }
    var readMemBlock = null;
    var chunkSz_ = 1024*1024;
    var currSz_ = 0;
   // var ext = input.files[fileNdx].name.split('.').pop();
    var totSz_ = input.files[fileNdx_].size; 
    load_file_cb = Module.cwrap('LdLasCppJS', 'number', ['arrayPointer', 'number', 'number']);
    var s_action_chunk = Module._malloc(128);
    Module.HEAPU8.set(new TextEncoder().encode("datchunk"), s_action_chunk); 
 
    var readerOnLoad = async function (e) {    
        var data = e.target.result;
        var dtsize = e.target.result.byteLength;
        var array = new Uint8Array(data);
        var res_ptr = Module._malloc(dtsize);
        Module.HEAPU8.set(array, res_ptr);  
        chunkSz_ = load_file_cb(res_ptr, 1, dtsize); 
        Module._free(res_ptr);
        currSz_ +=  dtsize;
    };

    var readerDoneLoad = async function (e) {
        if(chunkSz_=== 0){
            fileNdx_ = fileNdx_ + 1;
            if(fileNdx_ === numFiles_){
               post_proc_file_cb = Module.cwrap('PostProcessDataJS', 'number', ['number', 'number']);
               post_proc_file_cb(0,0);
               return ;
             }
            else{
                // Start reading new file
                totSz_ = input.files[fileNdx_].size; 
                currSz_ = 0;
                chunkSz_ = load_file_cb(s_action_chunk, 0, 1); 
            }
        }
        if(chunkSz_=== -1){
            post_proc_file_cb = Module.cwrap('PostProcessDataJS', 'number', ['number', 'number']);
            post_proc_file_cb(0,0);
            return ;
        }
        var rdp = Math.floor(100* currSz_/totSz_);
        document.getElementById('GFG').innerHTML = "Reading " + fileNdx_ + " from " + numFiles_+ " " + rdp + "%";
        readMemBlock(currSz_,chunkSz_);
    }

    var readerOnError = function (e) {
        console.log('Error : ' + e.type);
    };

     readMemBlock = async function(_offset, length){
        var reader = new FileReader();
        reader.onload = readerOnLoad;
        reader.onloadend  = readerDoneLoad;
        reader.onerror = readerOnError;    
        var blob = input.files[fileNdx_].slice(_offset, length + _offset);
        reader.readAsArrayBuffer(blob);
    }
    
    // Start reading
    chunkSz_ = load_file_cb(s_action_chunk, 0, 0); 
    readMemBlock(currSz_, chunkSz_);  
} //OnLoadLas



async function LoadLasAsync(input){
    const ppp = new Promise((aaa, rej) => {
        console.log(" INSIDE PROMISE");
        OnLoadLas(input);
    });
    return ppp;
}

function  doSomeLogging(){
    console.log("---Do logging----");
}

async function OnFileSelected(input) {
    console.log("HERE_I_AM-1");
    try{
        await  LoadLasAsync(input);
    }
    catch(error){
        console.log("error " + error);
    }
    //OnLoadLas(input);
    console.log("HERE_I_AM-2");
    //await PostProcess();

} //OnFileSelected

function OnSampleLoad(){
    console.log("-OnSampleLoad-");
}

function OnFileOpen() {
    document.getElementById('attachment').click();
}

/*
'2d'
'webgl'
'webgl2'
'experimental-webgl'
'bitmaprenderer'
*/
function resizeCanvas() {
    resize_cb = Module.cwrap('CallCFunc', 'number', ['number', 'number']);
    document.getElementById('canvas').height = window.innerHeight;
    document.getElementById('canvas').width = window.innerWidth;
    resize_cb(window.innerWidth, window.innerHeight);
}

function forceResize(){
    window.dispatchEvent(new Event('resize'));
}

function GetBkColorValue(){
    var vs = document.getElementById("bkcolor").value; 
    var vi =  "0x"+vs.slice(1);
    var v = parseInt(vi);
    return v;    
}

function GetBudgetValue(){
    return  document.getElementById("budVal").value; 
}

function GetPtSizeValue(){
    return document.getElementById("ptSize").value; 
}

function GetFovValue(){
    return document.getElementById("fovVal").value; 
}

function GetUIString(){
    return gIdChanged;
}

function SetColorMode(){
    gDisableRdb = 1;
}

function UpdateColorModeUI(){
    var el = document.getElementById("colrgbId");
    if(gDisableRdb === 1){
        console.log("-----here1===========*");
        //el.disabled = true;
        el.setAttribute("disabled", true);
    }else{
        console.log("-----here2===========*");
        el.removeAttribute('disabled');
        //el.disabled = false;
        //el.setAttribute("disabled", false);      
    }
}

function OnUIEvent1(input){
    //SetColorMode();
    gIdChanged = input.id;
    var elType = document.getElementById(input.id).type;
    if(elType == "checkbox"){
        gUIChangeCB(123,document.getElementById(input.id).checked );
    }else{
        gUIChangeCB(123, document.getElementById(input.id).value);
    }
    if( input.id == "ruler"){
        setRuler(document.getElementById(input.id).checked); 
    }
    // gray out some UI elements
    if( input.id == "camOrto"){
        fovEl = document.getElementById("fovVal");
        if(document.getElementById(input.id).checked==true){
            fovEl.setAttribute("disabled", true);
        }else{
            fovEl.removeAttribute('disabled');
        }
    }
    if( input.id == "rdAll"){
        budgetEl = document.getElementById("budVal");
        if(document.getElementById(input.id).checked==true){
            budgetEl.setAttribute("disabled", true);
        }else{
            budgetEl.removeAttribute('disabled');
        }
    }
}

function OnBkhanged(input){
    gIdChanged = 'bkgcol';
    var val = GetBkColorValue(); 
    gUIChangeCB(123,val);
}


class ProscessEventsClass {
    constructor() {
        this.ctrlOn = 0;
        this.mouseOn = 0;
        this.action = 0;
        this.posX = -1;
        this.posY = -1;
        this.isRuler = 0;
    };

    onMouseUp(e) {
        this.mouseOn = 0;
        this.posX = -1;
        this.posY = -1;
    }

    onMouseClick(e) {
        //console.log("onMouseClick button= "+ e.button);
    }

    onDbClick(e){
        console.log("DBClick ruler "+ this.isRuler);
        if(this.isRuler===1){
            OnTest(1);
        } else {
            gCamDbClickCB(e.clientX,e.clientY);
        }
    }

    onMouseDown(e) {
        this.mouseOn = 1;
        this.posX = e.clientX;
        this.posY = e.clientY;
        if(e.button==0){//left
            this.action = 1; //rotate
            if(this.isRuler===1){
               // gMouseClickCB(0,0);
            }
        }
        if(e.button==2){//right
            console.log("===btnCode="+ e.button);
            this.action = 2; //move
        }
    }

    onMouseMove(e) {
        if(this.isRuler===1){
           gMouseMoveCB( e.clientX,e.clientY);
        }
        if (this.mouseOn == 1) {
            let dx = e.clientX - this.posX;
            let dy = e.clientY - this.posY;
            let adx = (dx > 0) ? dx : -dx;
            let ady = (dy > 0) ? dy : -dy;
            if(this.action==1){// rotation
                if (adx > ady) {
                    let vx = adx * 10000 / window.innerWidth;
                    gCamRotCb((dx>0)?-1:1, 0, 0, vx);
                }
                else {
                    let vy = ady * 10000 / window.innerHeight;
                    gCamRotCb(0, (dy>0)?-1:1, 0, vy);
                }
            }
            if(this.action==2){// move
                gCamMoveCb(dx,dy);
            }
            this.posX = e.clientX;
            this.posY = e.clientY;
        }
    }

    onMouseWheel(e) {
    }

    onWheel(e) {
        if (e.deltaY > 0) {
            gCamRotCb(0, 1, 1, 0);
        } else {
            gCamRotCb(0, -1, 1, 0);
        }
    }

    onKeyUp(e) {
        if (e.key == "Control") {
            this.ctrlOn = 0;
        }
        if((e.key == "m")||(e.key == "M")){
            OnTest(1); // Select
        }
    }

    onKeyDown(e) {
        switch (e.key) {
            case "ArrowUp": gCamRotCb(0, 1, this.ctrlOn, 200); break;
            case "ArrowDown": gCamRotCb(0, -1, this.ctrlOn, 200); break;
            case "ArrowLeft": gCamRotCb(-1, 0, this.ctrlOn, 200); break;
            case "ArrowRight": gCamRotCb(1, 0, this.ctrlOn, 200); break;
            case "Control": this.ctrlOn = 1; break;
            case "r" : OnTest(-1); break;
            case "t" : OnTest(-2); break; 
        }
    }

    onButton(e){
        //console.log("onButton");
    }
}

var ProcessEvents = new ProscessEventsClass();

function setRuler(v){
    ProcessEvents.isRuler = (v === true) ? 1:0;
}

function onTestCheckClick(cb) {
    ch_cb = Module.cwrap('OnDebugCheckBox', 'number', ['number']);
    if (cb.checked) {
        ch_cb(1);
    } else {
        ch_cb(0);
    }
}

function OnTestButton() {

}

function OnLoaded() {
    // init canvas
    htmlCanvas = document.getElementById('myPics');
    context = htmlCanvas.getContext('2d');
    resizeCanvas();
}

function OnStart() {
    console.log("-OnStart-\n");
    gCamRotCb   = Module.cwrap('CameraRotateJS', 'number', 'number', 'number', 'number',['number']);
    gCamMoveCb  = Module.cwrap('CameraMoveJS', 'number', ['number','number']);
    gCamDbClickCB = Module.cwrap('CameraMoveDbClickJS', 'number', ['number','number']);
    gUIChangeCB = Module.cwrap('OnUIChangeJS', 'number', ['number', 'number']);
    gMouseMoveCB = Module.cwrap('MouseMoveJS', 'number', ['number', 'number']);
    gMouseClickCB =  Module.cwrap('MouseClickJS', 'number', ['number', 'number']);
    resizeCanvas();
    window.addEventListener('resize', resizeCanvas, false);
    window.addEventListener('keydown', (event) => { ProcessEvents.onKeyDown(event); }, false);
    window.addEventListener('keyup', (event) => { ProcessEvents.onKeyUp(event); }, false);
    const el = document.getElementById("canvas");
    el.addEventListener('click', (event) => { ProcessEvents.onMouseClick(event); }, false);
    el.addEventListener('mouseup', (event) => { ProcessEvents.onMouseUp(event); }, false);
    el.addEventListener('mousemove', (event) => { ProcessEvents.onMouseMove(event); }, false);
    el.addEventListener('mousedown', (event) => { ProcessEvents.onMouseDown(event); }, false);
    el.addEventListener('mousewheel', (event) => { ProcessEvents.onMouseWheel(event); }, false);
    el.addEventListener('wheel', (event) => { ProcessEvents.onWheel(event); }, false);
    el.addEventListener('dblclick', (event) => { ProcessEvents.onDbClick(event); }, false);

    const color = document.getElementById("bkcolor");
    color.addEventListener('input', function(e) {OnBkhanged(e);});
    // disable some elements
    //document.getElementById("colrgb").disabled = true;
 }



//OnStart();