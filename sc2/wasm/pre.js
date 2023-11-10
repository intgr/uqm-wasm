Module['preRun'] ||= [];
Module["preRun"].push(function () {
    addRunDependency('syncfs');

    FS.mkdir('/home/web_user/.uqm');
    FS.mount(IDBFS, {}, '/home/web_user/.uqm')
    FS.syncfs( /*populate=*/ true, err => {
        if (err)
            throw err;
        else {
            removeRunDependency('syncfs')
            console.log("FS Synced")
        }
    });
});