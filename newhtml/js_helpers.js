
  

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
var gIdChanged = "unknown"

function callWasm1() {
    console.log("CallWasm");
    Module['canvas'] = document.getElementById('myPics');
    const result = Module.ccall('CallCFunc', 'number');
}

function OnTest( v) {
    console.log("Hello TestMe JS-\n");
    test_cb = Module.cwrap('OnTestJS', 'number', ['number']);
    test_cb(v);
}

function OnMouseMove(e) {

    gv.mouseX = e.offsetX;
    gv.mouseY = e.offsetY;
    OnDraw();
}

function OnFileSelected(input) {
    document.getElementById('GFG').innerHTML = 'Reading ... '
    var file = input.files[0];
    if (!file) {
        console.log("NO FILE");
        return;
    }
    var readMemBlock = null;
    var chunkSz_ = 1024*1024;
    var currSz_ = 0;
    var ext = input.files[0].name.split('.').pop();
    var totSz_ = input.files[0].size;
    if(totSz_>=2147483648){
        alert('File exceeds size limit');
        return;
    }
    console.log("##### Reading file size ###### " + totSz_);
    load_file_cb = Module.cwrap('LoadFileDataJS', 'number', ['arrayPointer', 'number', 'number'], { async: true });

    var s_tmp = Module._malloc(128);
    array8 = new TextEncoder().encode("hdrsize");
    Module.HEAPU8.set(array8, s_tmp); 
    xaxa =load_file_cb(s_tmp, 10, 0); 
    console.log("xoxo="+xaxa);

    var s_ptr = Module._malloc(1);
    load_file_cb(s_ptr, 0, totSz_/2);

    var readerOnLoad = function (e) {      
        var data = e.target.result;
        var dtsize = e.target.result.byteLength;
        //console.log("#### Reading -->>>>>>>>> " + dtsize);
        var array = new Uint8Array(data);
        var res_ptr = Module._malloc(dtsize);
        Module.HEAPU8.set(array, res_ptr);  
        load_file_cb(res_ptr, 1, dtsize); // 1 =copy 
        Module._free(res_ptr);
        currSz_ +=  dtsize;
        //load_file_cb(res_ptr, 2, totSz_);
    };

    var readerDoneLoad = function (e) {
        if(currSz_ ==  totSz_){
            console.log("#### load done currSz_ ="+ currSz_); 
            var v_ptr = Module._malloc(1);
            load_file_cb(v_ptr, 2, totSz_);
        }else{
            var nextPos = currSz_ + chunkSz_;
            if( nextPos <= totSz_){
                var rdp = Math.floor(100* currSz_/totSz_);
                document.getElementById('GFG').innerHTML = "Reading " +  rdp + "%";
                readMemBlock(currSz_,chunkSz_);
            }else{
                console.log("#### load last currSz_ ="+ currSz_); 
                readMemBlock(currSz_, totSz_ - currSz_); 
            }
        }
    }

    var readerOnError = function (e) {
        console.log('Error : ' + e.type);
    };

    readMemBlock = function(_offset, length){
        var reader = new FileReader();
        reader.onload = readerOnLoad;
        reader.onloadend  = readerDoneLoad;
        reader.onerror = readerOnError;    
        //var blob = _file.slice(_offset, length + _offset);
        var blob = input.files[0].slice(_offset, length + _offset);
        reader.readAsArrayBuffer(blob);
    }

    //readMemBlock(currSz_,totSz_);
    readMemBlock(currSz_, chunkSz_);
  
}

function OnSampleLoad(){
    console.log("-OnSampleLoad-");
    //const req = new XMLHttpRequest();
    //req.open("GET", "https://drive.google.com/file/d/1HYSlnX1xQ79pwgYSU50yz7uguVv9iSpb/view?usp=sharing");
    //req.open("GET", "/sample.las", true);
    /*
    req.responseType = "arraybuffer";
     req.onload = (event) => {
        const arrayBuffer = req.response; // Note: not req.responseText
        if (arrayBuffer) {
            const byteArray = new Uint8Array(arrayBuffer);
                console.log(" here byteArray" + byteArray.length)
                byteArray.forEach((element, index) => {
             });
        }
    };
    req.send();
    */
   /*
    req.onload = function() {
        var content = req.responseText;
        console.log(" Download Size" + content.length);
         console.log("Yes Download " + content);
    };
    req.onerror = function() {
        alert("Download failure.");
    };
    req.send();
    */
}

function OnFileOpen() {
    document.getElementById('attachment').click();
}


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

function OnUIEvent1(input){
    gIdChanged = input.id;
    var elType = document.getElementById(input.id).type;
    if(elType == "checkbox"){
        gUIChangeCB(123,document.getElementById(input.id).checked );
    }else{
        gUIChangeCB(123, document.getElementById(input.id).value);
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
    };

    onMouseUp(e) {
        this.mouseOn = 0;
        this.posX = -1;
        this.posY = -1;
    }

    onMouseClick(e) {
    }

    onDbClick(e){
        console.log("DBclick: x= " + e.clientX + " y="+e.clientY);
        gCamDbClickCB(e.clientX,e.clientY);
    }

    onMouseDown(e) {
        this.mouseOn = 1;
        this.posX = e.clientX;
        this.posY = e.clientY;
        if(e.button==0){//left
            this.action = 1; //rotate
        }
        if(e.button==2){//right
            console.log("btnCode="+ e.button);
            this.action = 2; //move
        }
    }

    onMouseMove(e) {
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