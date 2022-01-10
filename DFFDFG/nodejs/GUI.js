
var http = require('http');
const fs = require('fs');
//const bash = require("./bash.js")

var dir = '/home/dockerFileGenerator/';
const { spawn } = require('child_process');
var structure= [];
const regex=/^[0-9]{1,3}_.*/;

var mergefoldername="merge";
var temporarypath="/sharedFolder/temp";
var configpath="/sharedFolder/configurationFile.txt";


function dockerinstall()
{
    let intelement;
    let startpath;
    let tempname;
    let finalname;
    let endpath;
    let category;
    let correctformat;
    let filename="randomoutente?";
    let actions={};
    let position=1;
    http.createServer(function (req, res) 
{

    res.writeHead(200, {'Content-Type': 'text/html'});
    res.write(html);
    if (req.method == 'POST') 
    {
        var jsonString = '';

        req.on('data', function (data) 
           {
            jsonString += data;
           });

        req.on('end', function () 
           {
                functions=jsonString.split("&");
                functions.forEach(element => 
                    {
                        splittedfunc=element.split("=");
                        console.log(element);
                        correctformat=splittedfunc[1].split("%2F").join("/");
                        correctformat=correctformat.split("END").join("");
                        category=correctformat[0];
                        if (category=="0")
                        {
                            actions[category]=actions[category]+"|"+correctformat;
                        }
                        else 
                        {
                            actions[category]=correctformat;
                        }
                    });
                for (element in actions)
                {
                          
                    if (element=="0")
                        {
                                {
                                    let moment=actions[element].split("|");

                                for (let c=1; c<moment.length; c=c+1)
                                {
                                    tempname=moment[c].split("/")[0]+"docker";
                                    finalname=moment[c].split("/")[0]+"final";
                                    console.log("Eseguo runMe di "+moment[c]+"passando come parametri ["+tempname+","+finalname+","+temporarypath+","+configpath+"]");
                                }
                                }
                           
                        }
                    else
                    {
                        if (element=="1")
                        {
                            console.log("Eseguo runMe di "+actions[element]+"passando come parametri ["+mergefoldername+"]");
                        }
                        else if (element=="2")
                        {
                            console.log("Eseguo runMe di "+actions[element]+"passando come parametri ["+mergefoldername+","+mergefoldername+actions[element].split("/")[0]+","+actions[element].split("/")[0]+"docker"+","+temporarypath+","+configpath+"]");
                        }
                      else if (element=="3")
                        {
                            console.log("Eseguo runMe di "+actions[element]+"passando come parametri ["+actions[element-1].split("/")[0]+"]");
                        }
                        
                    }
                    

                    
                    intelement=parseInt(element);

                    if ((intelement+1) in actions)
                    {
                        if (intelement==0)
                        { // Zero element might be more than one, and are stored in a string, so we must know which of them we have to copy
                            endpath=actions[intelement+1];

                            moment=actions[element].split("|");
                             for (let c=1; c<moment.length; c=c+1)
                                {
                                    finalname=moment[c].split("/")[0]+"final";
                                    startpath=moment[c]+"/"+finalname;

                                     console.log("Copio tutto da "+startpath+" ==> "+endpath);
                                     //call.bashmove(startpath,endpath);

                                }

                             //FILENAME E DA DEFInire ASSOLUTAMENTE

                        }
                        else{
                               startpath=actions[intelement]+"/"+filename; //FILENAME E DA DEFInire ASSOLUTAMENTE
                              endpath=actions[intelement+1];
                         console.log("Copio tutto da "+startpath+" ==> "+endpath);

                        //call.bashmove(startpath,endpath);

                        }

                    }
                    else {
                    console.log("Copio tutto nella cartella finale");
                        //bashmove(startpath,finalfolderpath);
                    }
                    
                }
                        

                        
            

          });
     }
    
        res.end();
    }).listen(3000);
}
//This is a recursive function that load the folder structure of the software in var structure containing the path of each folder,going deep until find the file runMe.sh


function folderstructure(path,deep)
{
    let localpath=dir+path
    const localfiles = fs.readdirSync(localpath);
    if((localfiles.find((file)=>file === "runMe.sh"))==="runMe.sh") // Find a more efficient method
    {
        mom=structure.pop();
        structure.push(mom+"END");
    }
    else
    {
     localfiles.forEach
        (
            file => 
            {
                let completepath=localpath+"/"+file;
                let partialpath=path+"/"+file;
                let state=fs.statSync(completepath);
                if (state.isDirectory()) // QUESTO CAMBIALO CAZZO
                {
                  if ((deep==0 && file.match(regex) )|| (deep!=0 && file[0]!="."))  
                  {
                      structure.push(partialpath);
                      folderstructure(partialpath,(deep+1));
                  }
                }
            }
        );   
    }
}

function loadexceptions()
{
    let file = fs.readFileSync('/nodejs/exceptions.txt');
    let reading = file.toString();
    reading= reading.replace(/[\n\r]/g, '');
    let exception;
    let exceptionstring="<script> var exceptionstruct ={}; ";
    let exceptions= reading.split("#");
    exceptions.forEach(element =>
                       {
                            exception=element.split("=");
                            exceptionstring=exceptionstring+ ' exceptionstruct["'+exception[0]+'"]="'+exception[1]+'"; ';

                        }
                      
                      );
    exceptionstring=exceptionstring+'</script>';
    return exceptionstring
}


function folderlayout()
{
   let prevlen=0;
   let html="";
   let l,sl;
   let nextl="dif",nextsl; //only to make it different from l and sl

    
    for (let i=0; i<structure.length; i++) 
    {

        const path = structure[i].split("/");
        path.shift();
        let len = path.length; 
        
        // OPEN MARK SECTION where l sl nextl and next sl are the previous because arent yet updsated
        if (l!=nextl && l!=0)
            {
                html=html+"<span class='container'><span class='subcontainer'>";
            } 
        else if (sl!=nextsl)
            {
                html=html+"<span class='subcontainer'>";
            }
        if (path[path.length-1].endsWith("END"))
            {
                input=path[path.length-1];
                input=input.substring(0, input.length - 3);
                html=html+"<span class='options' id='option"+input+"'><input type='checkbox' id='"+input+"'  onchange='grey(\""+path[0]+"\")' name ='checkbox' class='"+path[0]+"' value='"+path.join("/")+"'>"+input+"</span>";               
            }
        else
            {
                html=html+"<span class='content n"+len+"'>";
                html=html+"<span class='title'>"+path[path.length-1]+"</span>";
            }
        let mom=path[0].split("_");
        l=mom[0];
        sl=mom.slice(1,mom.length).join("_");

        
        // CLOSE MARK SECTION -- based on next folder "line"
        if ((i+1)!=structure.length)
            {
                const nextpath = structure[i+1].split("/");
                nextpath.shift();   
                let nextmom=nextpath[0].split("_");
                nextl=nextmom[0];
                //to avoid that other _ after the first can "broke the code" i just join them later
                
                nextsl=nextmom.slice(1,nextmom.length).join("_");
                if (nextl!=l)
                {
                    for (let x = 0; x < len-1; x++) 
                        {
                            html=html+"</span>";
                        }   
                     html=html+"</span></span><span class='separator'></span>";

                }
                else if (nextsl!=sl)
                {
                    for (let x = 0; x < len-1; x++) 
                        {
                            html=html+"</span>";
                        }   
                    html=html+"</span>";
                }
                else 
                {

                    let nextlen = nextpath.length; 
                    if (nextlen<len)
                        {
                            for (let x = 0; x < len-nextlen; x++) 
                                {
                                    html=html+"</span>";
                                }   
                        }
                }
            } 
            else
            {
                html=html+"</span></span>";
            }
    }

 
return html;
}

let html;
var file = fs.readFileSync('/nodejs/website/start.html');
var start = file.toString();
file = fs.readFileSync('/nodejs/website/end.html');
var end = file.toString();

folderstructure("",0);
var exceptions=loadexceptions();

html=start+exceptions;
html=html+file;
html=html+folderlayout();
html=html+end;
funcobj={};
let category;
dockerinstall();
