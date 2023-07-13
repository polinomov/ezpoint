
//const myPics = document.getElementById('myPics');
//const context = myPics.getContext('2d');
// document.getElementById("radioButtonX").disabled = true;
var htmlCanvas;
var context;
class GlobVars {
    mouseX = 0;
    mouseY = 0;
}
var gv = new GlobVars();

var gCamRotCb;
var gCamMoveCb;

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
    document.getElementById('GFG').innerHTML = 'Reading: ' + input.files[0].name;
    var file = input.files[0];
    if (!file) {
        console.log("NO FILE");
        return;
    }
    var ext = input.files[0].name.split('.').pop();
    console.log("YES FILE size=" + input.files[0].size + " ext=" + ext);
    var fz = input.files[0].size;
    var reader = new FileReader();
    reader.onload = function (e) {
        var data = reader.result;
        var array = new Uint8Array(data);
        var res_ptr = Module._malloc(fz);
        Module.HEAPU8.set(array, res_ptr);
        file_cb = Module.cwrap('FileBinDataJS', 'number', ['arrayPointer', 'number', 'number'], { async: true });
        switch (ext) {
            case "xyz": file_cb(res_ptr, fz, 0); break;
            case "las": file_cb(res_ptr, fz, 1); break;
        }
        Module._free(res_ptr);
    };
    reader.onloadend = function (e) {
        console.log("load done");
    }
    reader.onerror = function (e) {
        console.log('Error : ' + e.type);
    };
    reader.readAsArrayBuffer(input.files[0]);
}

function OnFileOpen() {
    document.getElementById('attachment').click();
}


function resizeCanvas() {
    // console.log("Resize w=" + window.innerWidth + " h=" + window.innerHeight);
    resize_cb = Module.cwrap('CallCFunc', 'number', ['number', 'number']);
    resize_cb(window.innerWidth, window.innerHeight);
}

function OnFovChanged(){
    var v = document.getElementById("fovVal").value; 
    ui_cb = Module.cwrap('OnUIChangeJS', 'number', ['number', 'number']);
    ui_cb(1,v);
}

function OnBudgetChanged(){
    var v = document.getElementById("budVal").value; 
    ui_cb = Module.cwrap('OnUIChangeJS', 'number', ['number', 'number']);
    ui_cb(2,v);
}

function OnPtSizeChanged(){
    var v = document.getElementById("ptSize").value; 
    ui_cb = Module.cwrap('OnUIChangeJS', 'number', ['number', 'number']);
    ui_cb(3,v);
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
    gCamRotCb =  Module.cwrap('CameraRotateJS', 'number', 'number', 'number', 'number',['number']);
    gCamMoveCb = Module.cwrap('CameraMoveJS', 'number', ['number','number']);
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
    //el.addEventListener('button', (event) => { ProcessEvents.onButton(event); }, false);
}



//OnStart();