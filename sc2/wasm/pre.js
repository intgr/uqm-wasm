Module.preRun ||= [];
Module.preRun.push(function () {
    window.wasm_syncfs = () => {
        FS.syncfs( /*populate=*/ false, err => {
            if (err) {
                alert("Saving to IndexedDB failed, saved game & preferences will not be persistent.");
            } else {
                console.log("Saved files to browser IndexedDB.");
            }
        });
    };

    addRunDependency("syncfs");

    FS.mkdir("/home/web_user/.uqm");
    FS.mount(IDBFS, {}, "/home/web_user/.uqm");
    FS.syncfs( /*populate=*/ true, err => {
        if (err) {
            alert("Populating from IndexedDB failed, saved game & preferences will not be persistent.");
        } else {
            removeRunDependency("syncfs")
            console.log("Loaded files from browser IndexedDB.")
        }
    });
});