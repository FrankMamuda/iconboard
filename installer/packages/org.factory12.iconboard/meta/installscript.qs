function Component() {
}

Component.prototype.isDefault = function() {
    return true;
}

Component.prototype.createOperations = function() {
    try {
        component.createOperations();
    } catch ( e ) {
        print( e );
    }

    if ( installer.value( "os" ) === "win" ) {
        component.addOperation( "CreateShortcut", "@TargetDir@/IconBoard.exe", "@DesktopDir@/IconBoard.lnk");
    }
}
