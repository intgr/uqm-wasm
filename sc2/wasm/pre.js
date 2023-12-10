Module.preRun ||= [];

Module.preRun.push(function () {
    let persistRequested = false;

    function requestPersistentStorage() {
        if (navigator.storage && navigator.storage.persist) {
            return navigator.storage.persist().then(isPersisted => {
                if (isPersisted) {
                    console.log("Persistent storage request was accepted.");
                } else {
                    throw "Persistent storage request was rejected.";
                }
            });
        } else {
            return Promise.reject("Persistent storage not supported by browser.");
        }
    }

    // Called from C code: uio_fclose()
    window.wasm_syncfs = () => {
        FS.syncfs( /*populate=*/ false, err => {
            if (err) {
                alert("Saving to IndexedDB failed, saved game & preferences will not be persistent.\n" + err);
            } else {
                console.log("Saved files to browser IndexedDB.");
            }
        });

        if (!persistRequested) {
            requestPersistentStorage().catch(err => {
                alert("Warning: Browser may delete saved games & preferences after inactivity.\n" + err);
            });
            persistRequested = true;
        }
    };

    addRunDependency("syncfs");

    FS.mkdir("/home/web_user/.uqm");
    FS.mount(IDBFS, {}, "/home/web_user/.uqm");
    FS.syncfs( /*populate=*/ true, err => {
        if (err) {
            alert("Populating from IndexedDB failed, saved game & preferences will not be persistent.\n" + err);
        } else {
            removeRunDependency("syncfs");
            console.log("Loaded files from browser IndexedDB.");
        }
    });
});
