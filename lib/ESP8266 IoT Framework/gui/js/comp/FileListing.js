import React, { useState, useEffect, version } from "react";
import PropTypes from "prop-types";

import styled from "styled-components";

import { Fetch, Flex, RedButton, Button, buttonStyle, cPrimary, Alert, Spinner } from "./UiComponents";
import { FiFile as File, FiTrash2 as Trash2, FiDownload as Download } from "react-icons/fi";

import Config from "../configuration.json";
let loc;
if (Config.find(entry => entry.name === "language")) {
    loc = require("./../lang/" + Config.find(entry => entry.name === "language").value + ".json");
} else {
    loc = require("../lang/en.json");
}

const Layout = styled.div`
    * {
        width:calc(660px + 3em);
        margin-left:0px;
        max-width:calc(100% - 40px);
    }
`;


const FileLine = styled(Flex)`
    padding:0.35em 0em;
    border-bottom:1px solid #ddd;
    
    :last-child 
    {
        border-bottom:0px;
    }

    &.selectable {
        span {
            text-decoration:underline;
        }
        cursor:pointer;
        padding-left:0.35em;
        padding-right:0.35em;
    }

    &.selectable:hover {
        background-color:#ff00cc11;
    }

    span
    {
        margin-left:0.6em;
    }

    Button
    {
        padding:0.4em 0.5em;        
    }

    div:first-child
    {
        padding:0.4em 0em;
    }

    svg 
    {
        vertical-align: -0.15em;
    }

    @media (max-width: 500px) 
    {
        flex-wrap:wrap;
        div:first-child {
            width:100%;
            margin-bottom:0.4em;
        }
    }
`;


export function FileListing(props) {
    const [state, setState] = useState({ files: [], used: 0, max: 0});
    const [versionInfo, setVersionData] = useState({ firmwareurl: "",platformversion:"",frameworkversion:"",sdkversion:"",libversion:""});

    useEffect(() => {
        fetchData();
    }, []);

    function fetchData() {
        fetch(`${props.API}/api/files/get`)
            .then((response) => {
                return response.json();
            })
            .then((data) => {
                fetch(`${props.API}/api/firmwareurl/get`)
                .then((response) => {
                    return response.json();
                })
                .then((versiondata) => {
                    setVersionData(versiondata);
                    setState(data);
                    console.log("Filesdata:",data);
                    console.log("FileListing Versiondata",versiondata);
                });
            });
            // console.log("Firmware Url:",versionInfo.firmwareurl);
            // console.log("Filesdata:",state);
    }

    let list;
        
    if (state.max > 0) {
        let filtered = 0;
        // console.log("Filelisting state:",state);

        if (state.files != null) {
            for (let i = 0; i < state.files.length; i++) {
                if (typeof props.filter === "undefined" || state.files[i].substr(state.files[i].length - (props.filter.length + 1)) == `.${props.filter}`) {
                    filtered++;
                }
            }
        }

       if (filtered == 0) {
            // console.log("Filelisting filtered =0");
            list = <FileLine><div>{loc.filesEmpty}</div></FileLine>;
        } else {
            // console.log("Filelisting filtered:",filtered);
            for (let i = 0; i < state.files.length; i++) {
                if (typeof props.filter === "undefined" ||
                    state.files[i].substr(state.files[i].length - (props.filter.length + 1)) == `.${props.filter}`) {       
                    const name = state.files[i];         
                    list = <>{list}                        
                        <FileLine className={props.selectable ? "selectable" : ""} onClick={() => {if (typeof props.onSelect !== "undefined") {props.onSelect(name);}}}>
                            <div><File /><span>{state.files[i]}</span></div>
                            <div>
                                <a href={`${props.API}/download/${state.files[i]}`} rel="noreferrer" target="_blank" onClick={(e) => { e.stopPropagation();}}>
                                    <Button title={loc.filesDl}><Download /></Button>
                                </a>                
                                <Fetch href={`${props.API}/api/files/remove?filename=${state.files[i]}`} POST onFinished={fetchData}>
                                    <RedButton title={loc.filesRm} ><Trash2 /></RedButton>
                                </Fetch>   
                            </div>
                        </FileLine></>;
                }
            }   
        }  
    } else {
        list = <FileLine><div><Spinner /></div></FileLine>;
    }

    let header;
    if (props.selectable) {
        header = loc.filesFwTitle + ":";
    } else {
        header = loc.filesTitle;
    }

    // hier bauen wir uns den HTML code zusammen der für die Aufzaehlung der Libaries am Ende ausgegeben werden soll
    var LibArray = versionInfo.libversion.split(",");
    let libItems;
    for (var x = 0; x < LibArray.length; x++) {
        var newstr = LibArray[x].replace(/'/g,"");
        // console.log(newstr);
        var strLib = newstr.split(":");
        libItems = <>{libItems}<Layout>{strLib[0]}:{strLib[1]}<br/></Layout></>;
    }


    return <><Flex>
        <div><Upload action={`${props.API}/upload`} onFinished={fetchData} filter={props.filter} /></div>
        {parseInt(state.max) > 0 ? <div>{Math.round(state.used / 1000)} / {Math.round(state.max / 1000)} kB {loc.filesUsed}</div> : ""}
    </Flex><h3>{header}</h3>{list}
    <><Layout><hr /></Layout></>
    <><Layout><h3>{loc.VersionenTitle}</h3></Layout></>
    <><Layout><p>React (preact 10.26.2):{version}</p></Layout></>
    <><Layout><p>Platform:{versionInfo.platformversion}</p></Layout></>
    <><Layout><p>Adruino-Espressif-Framework:{versionInfo.frameworkversion}</p></Layout></>
    <><Layout><p>ESP-Espressif-SDK:{versionInfo.sdkversion}</p></Layout></>
    <><Layout><h4>{loc.LibraryTitle}</h4></Layout></>
    <><Layout>{libItems}</Layout></>
    </>;
}

/*
       <><Layout><hr /></Layout></>
       <><Layout><h3>{loc.VersionenTitle}</h3></Layout></>
       <><Layout><p>React (preact 10.26.2):{version}</p></Layout></>
       <><Layout><p>Platform:{versionInfo.platformversion}</p></Layout></>
       <><Layout><p>Adruino-Espressif-Framework:{versionInfo.frameworkversion}</p></Layout></>
       <><Layout><p>ESP-Espressif-SDK:{versionInfo.sdkversion}</p></Layout></>
       <><Layout><h4>{loc.LibraryTitle}</h4></Layout></>
       <><Layout>{libItems}</Layout></>

*/


    // return <><h3>{header}</h3><Flex>
    //     {<div>{state.firmwareurl}</div>}
    // </Flex>
    // <Flex>
    //      <div><Button onClick={() => { setFwUpdate(true);}}>{loc.firwmwareUpdateBtn}</Button></div> 
    //  </Flex>
    //  <><Layout><hr /></Layout></>
    //  <><Layout><h3>{loc.VersionenTitle}</h3></Layout></>
    //  <><Layout><p>React (preact 10.26.2):{version}</p></Layout></>
    //  <><Layout><p>Platform:{state.platformversion}</p></Layout></>
    //  <><Layout><p>Adruino-Espressif-Framework:{state.frameworkversion}</p></Layout></>
    //  <><Layout><p>ESP-Espressif-SDK:{state.sdkversion}</p></Layout></>
    //  <><Layout><h4>{loc.LibraryTitle}</h4></Layout></>
    //  <><Layout>{libItems}</Layout></>
    //  <Confirmation active={FwUpdate}
    // confirm={() =>{setFwUpdate(false);DoFirmwareUpdate();}}
    // cancel={() => setFwUpdate(false)}>{loc.firwmwareUpdateQuest}</Confirmation>
    //  </>; 




FileListing.propTypes = {
    API: PropTypes.string,
    onSelect: PropTypes.func,
    filter: PropTypes.string,
    selectable: PropTypes.bool,
};


function Upload(props) {
    const [state, setState] = useState("");

    let status;
    if (state == "busy") {status = <><Spinner /></>;} else {status = <>{loc.filesBtn}</>;}

    const render =
        <><form action={props.action} method="post" name="upload" encType="multipart/form-data">
            <FileLabel id="uploadLabel" className={state}>{status}<input type="file" id="file"
                onClick={(e) => {
                    if (state == "busy") {
                        e.preventDefault();
                    }
                }}
                onChange={(e) => {
                    const form = document.forms.namedItem("upload");
                    const files = e.target.files;
                    const formData = new FormData();

                    if (files.length > 0) {
                        if (typeof props.filter === "undefined"
                            || files[0].name.substr(files[0].name.length - (props.filter.length + 1)) == `.${props.filter}`) {
                            setState("busy");

                            formData.append("myFile", files[0]);

                            fetch(form.action, {
                                method: "POST",
                                body: formData,
                            }).then((response) => { return response.json(); })
                                .then((data) => {
                                    if (data.success == true) {
                                        setState("ok");
                                        props.onFinished();
                                    } else {
                                        setState("nok");
                                    }
                                });
                        } else {
                            setState("wrongtype");
                        }

                    }
                }} />
            </FileLabel>
        </form>
        <Alert active={state == "nok"}
            confirm={() => setState("")}>
            {loc.filesMsg1}</Alert>
        <Alert active={state == "wrongtype"}
            confirm={() => setState("")}>
            {loc.filesMsg2} (.{props.filter})</Alert>
        </>;
    return render;
}

const FileLabel = styled.label`
    ${buttonStyle}  

    display:inline-block;
    width:100px;
    text-align:center;
    
    @media (max-width: 500px) 
    {
        width:90px; 
    }

    &.busy 
    {
        cursor: default;
        :hover
        {
            background-color: ${cPrimary};
        }
    }

    svg {
        width:1.2em;
        height:1.2em;
        vertical-align:-0.25em;
    }

    input[type="file"] {
        display: none;
    } 
`;