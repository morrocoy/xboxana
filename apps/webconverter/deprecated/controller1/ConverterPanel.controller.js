sap.ui.define([
   'sap/ui/jsroot/GuiPanelController',
   'sap/ui/model/json/JSONModel'
], function (GuiPanelController, JSONModel) {
   "use strict";

   return GuiPanelController.extend("localapp.controller.TestPanel", {

      // function called from GuiPanelController
      onPanelInit : function() {
         var id = this.getView().getId();
         // console.log("Initialization TestPanel id = " + id);
         // such data will be produced on server from TFitPanelModel
         var model = new JSONModel({
            fDataNames:[ { fId:"1", fName: "----" } ],
            fSelectDataId: "0",
            fModelNames: [ { fId:"1", fName: "----" } ],
            fSelectModelId: "0"
         });
         this.getView().setModel(model);
      },

      // function called from GuiPanelController
      onPanelExit : function() {
      },
      

      OnWebsocketMsg: function(handle, msg, offset) {
         if (typeof msg != "string") {
            // console.log('TestPanel ArrayBuffer size ' +  msg.byteLength + ' offset ' + offset);
            var arr = new Float32Array(msg, offset);

            this.getView().byId("SampleText").setText("Got binary as float array\n" + 
                  'array length ' + arr.length + '\n' +
                  ' [0] = ' + arr[0] + '\n' +
                  ' [7] = ' + arr[7] + '\n' + 
                  ' [last] = ' + arr[arr.length-1]);

            return;
         }

         if (msg.indexOf("MODEL:")==0) {
            var json = msg.substr(6);
            var data = JSROOT.parse(json);

            this.getView().byId("SampleText").setText("Model size:" + json.length);

            if (data)
               this.getView().setModel(new JSONModel(data));

         } else {
            this.getView().byId("SampleText").setText("Get message:\n" + msg);
         }
      },
      
      handleGetBinary: function() {
         // just request binary data
         if (this.websocket)
            this.websocket.Send("GET_BINARY");
      },

      handleFitPress : function() {
         // To now with very simple logic
         // One can bind some parameters direct to the model and use values from model
         var v1 = this.getView().byId("FitData"),
             v2 = this.getView().byId("FitModel");

         if (this.websocket && v1 && v2)
            this.websocket.Send('DOFIT:"' + v1.getValue() + '","' + v2.getValue() + '"');
         console.log("test!" + v1);
      }

   });

});
