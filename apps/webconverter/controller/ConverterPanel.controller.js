sap.ui.define([
   'sap/ui/jsroot/GuiPanelController',
   'sap/ui/model/json/JSONModel',
   'sap/ui/core/Fragment'
], function (GuiPanelController, JSONModel) {
   "use strict";

   return GuiPanelController.extend("localapp.controller.TestPanel", {

      // function called from GuiPanelController
      /*onPanelInit : function() {
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
      onClose : function() {
      },*/
	   
	  //function called from GuiPanelController
      onPanelInit : function() {
         var id = this.getView().getId();
         var opText = this.getView().byId("idOutputFileName");
         var data = {
				fSelectStructureId: "0",
				fDateBegin: "2018/03/01",
				fDateEnd: "2018/03/31",
				fOutputFileName: "output.root",
				fFileFormatId: 0,
			 	fConvertEnabled:true,
			    fLog:""
         };
         this.getView().setModel(new JSONModel(data));
         this._data = data;     
      },
      
	  // Assign the new JSONModel to data      
      OnWebsocketMsg: function(handle, msg, offset) {
		 //this.getView().byId("SampleText").setText("Get message:\n" + msg);
		  
         if(msg.startsWith("sigSetModel:")){
			var json = msg.substr(msg.indexOf(":")+1);
            var data = JSROOT.parse(json);
            if(data) {
               this.getView().setModel(new JSONModel(data));
               this._data = data;
               this.copyModel = JSROOT.extend({},data);
            }     
         }
         else if(msg.startsWith("sigEnableConvert:")){
			var json = msg.substr(msg.indexOf(":")+1);
            var data = JSROOT.parse(json);
            if(data) {
               this.getView().setModel(new JSONModel(data));
               this._data = data;
               this.copyModel = JSROOT.extend({},data);
            }     
         }
		  
		  
      },
	   /*
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
      },*/
      
      onCancel: function() {
         // just request binary data
		   //this.getView().byId("SampleText").setText("Convert\n");
         if (this.websocket)
            this.websocket.Send("GET_BINARY");
		 console.log("Hallo1!"); 
      },

      onConvert : function() {
         // To now with very simple logic
         // One can bind some parameters direct to the model and use values from model
         //var v1 = this.getView().byId("idStructure"),
         //    v2 = this.getView().byId("idStartDate");
         //if (this.websocket && v1 && v2)
         // this.websocket.Send('DOFIT:"' + v1.getValue() + '","' + v2.getValue() + '"');
         
		 if (this.websocket)
            this.websocket.Send('sigOnConvert:' + this.getView().getModel().getJSON());
		 
		 //this.getView().byId("SampleText").setText('DOFIT:"' + this.getView().getModel().getJSON());
      }

   });

});
