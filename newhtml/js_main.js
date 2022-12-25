
//const myPics = document.getElementById('myPics');
//const context = myPics.getContext('2d');

var htmlCanvas;
var context;
class GlobVars {
    mouseX = 0;
    mouseY = 0;
}
var gv = new GlobVars();

function TestMe() {
    console.log("Hello\n");
    //alert("Warning");
   // htmlCanvas = document.getElementById('myPics');
    //context = htmlCanvas.getContext('2d');
    context.fillStyle = "red";
    context.fillRect(100, 100, 100, 100);
}

function OnDraw( ) {
    context.fillStyle = "black";
    context.fillRect(0, 0, htmlCanvas.width, htmlCanvas.height);
    //context.font = "10px Arial";
    context.fillStyle = "white";
    context.fillText("x= " + gv.mouseX + " y=" + gv.mouseY + " ct=" + htmlCanvas.offsetTop, 30, 60);
    context.fillRect(gv.mouseX, gv.mouseY, 10, 10);
}

function OnMouseMove(e) {
    
    //console.log("move" + e.offsetX);
    gv.mouseX = e.offsetX;
    gv.mouseY = e.offsetY;
    OnDraw();
}

function OnFileSelected(input) {
    console.log("fileSelected-- " + input.files[0].name);
    var file = input.files[0];
    if (!file) {
        console.log("NO FILE");
        return;
    }
    console.log("YES FILE size=" + input.files[0].size);
    var reader = new FileReader();
    reader.onload = function (e) {
        // binary data
        //console.log(e.target.result);
        console.log("done  reading");
    };
    reader.onerror = function (e) {
        // error occurred
        console.log('Error : ' + e.type);
    };
    reader.readAsArrayBuffer(input.files[0]);  
}

function OnFileOpen() {
    document.getElementById('attachment').click();
}


function resizeCanvas() {
    htmlCanvas.width = window.innerWidth-20;
    htmlCanvas.height = window.innerHeight-50;
    OnDraw();
    console.log("Resize");
}

function OnLoaded() {  
    // init canvas
    htmlCanvas = document.getElementById('myPics');
    context = htmlCanvas.getContext('2d');
    resizeCanvas();
 }

function OnStart()
{
    console.log("OnStart\n");
    window.addEventListener('mousemove', (e) => { OnMouseMove(e) });
    window.addEventListener('resize', resizeCanvas, false);
    window.addEventListener('DOMContentLoaded', (e) => { OnLoaded() });
}

OnStart();