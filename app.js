'use strict'

// C library API
const ffi = require('ffi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');
const mysql = require('mysql');
var connection = null;

app.use(fileUpload());

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});


// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }
 
  let uploadFile = req.files.uploadFile;
 
  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }

    res.redirect('/');
  });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    console.log(err);
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      res.send('');
    }
  });
});
 
//******************** Your code goes here ******************** 

var MyLibrary = ffi.Library('./parser/gedcomLib.so', {

  "filetoJSON": [ 'string', [ 'string'] ],
  "getIndiList" : ['string', ['string' ] ],
  "simpleGED" : ['void', ['string','string']],
  "addIndi" : ['void', ['string','string']]
});

app.get('/simpleGED/', function(req, res){
	
	var obj = req.query.obj;
	var fileName = req.query.fileName;
	MyLibrary.simpleGED(JSON.stringify(obj), fileName);
	res.send(obj);

});
 
app.get('/addIndi/', function(req, res){
	var obj = req.query.obj;
	var fileName = req.query.fileName;
	console.log(fileName);
	MyLibrary.addIndi(JSON.stringify(obj), fileName); 
	res.send(obj);
});

app.get('/uploads/', function(req, res){
    var result = fs.readdirSync("./uploads/");
    var rArray = [];
    for(var i = 0; i < result.length; i++){
      var filePath = "./uploads/" + result[i];
      var jsonResult = MyLibrary.filetoJSON(filePath);
	  var obj = JSON.parse(jsonResult)
	  obj.fileName = result[i];
	  rArray.push(obj);
    }
    res.send(rArray);
});

app.get('/indiList/', function(req,res){
	
	console.log(req.query.fileName);
	var result = MyLibrary.getIndiList(req.query.fileName);
	var obj = JSON.parse(result);
	res.send(obj);
	 
});
app.get('/connect/', function(req,res){
	connection = mysql.createConnection({
		host: 'dursley.socs.uoguelph.ca',
		user: req.query.userName,
		password: req.query.password,
		database : req.query.database
	});
	connection.connect(function(err){
		
			if(err){
				var toReturn = {"type" : "error"};
				res.send(toReturn);
			}else{
				var table1 = 'create table IF NOT EXISTS  FILE (file_id INT PRIMARY KEY AUTO_INCREMENT, file_Name VARCHAR(60), source VARCHAR(250), version VARCHAR(10), encoding VARCHAR(10), sub_name VARCHAR(62), sub_addr VARCHAR(256), num_individials int, num_families int);';
				var table2 = 'create table IF NOT EXISTS INDIVIDUAL (ind_id int PRIMARY KEY AUTO_INCREMENT, surname VARCHAR(256) NOT NULL, given_name varchar(256) not null, sex varchar(1), fam_size int, source_file INT, foreign key(source_file) references FILE(file_id) on delete cascade);'
				connection.query(table1, function(err, result){
					if (err)
						console.log(err);
					else
						console.log("Table Created");
				});
				connection.query(table2, function(err, result){
					if (err)
						console.log(err);
					else
						console.log("Table Created");
				});
				toReturn = {"type" : "ok"};
				res.send(toReturn);
			}
	});
});


app.get('/query/', function(req, res){
	connection.query(req.query.query, function (err, rows, fields){
		if(err)
			console.log("Error " + err);
		else{
			console.log("Success for " + req.query.query);
			console.log(rows);
			res.send(rows);
		}
		
	});
});

//Sample endpoint
app.get('/someendpoint', function(req , res){ 
  res.send({
    foo: "bar"
  });
});

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);
