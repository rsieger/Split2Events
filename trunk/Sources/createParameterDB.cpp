#include "Application.h"

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************
// https://store.pangaea.de/software/Split2Events/ParameterDDI.txt
//
// Split2Events: 8
//
// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************
// 2012-07-05

int MainWindow::getNumParameterQuerys()
{
    int             n                      = 0; // Number of lines
    int             i_NumOfParameterQuerys = 0; // Number of parameter querys

    QStringList     sl_Input;

// **********************************************************************************************
// get number of parameter querys

    removeFile( getParameterDDIFilename() );

    downloadFile( findCurl(), QLatin1String( "https://store.pangaea.de/software/Split2Events/ParameterDDI.txt" ), getParameterDDIFilename() );

    if ( ( n = readFile( getParameterDDIFilename(), sl_Input ) ) > 0 ) // System encoding
    {
        if ( sl_Input.at( 0 ).startsWith( "Split2Events" ) == true )
            i_NumOfParameterQuerys = sl_Input.at( 0 ).section( ": ", 1, 1 ).toInt();
    }

    return( i_NumOfParameterQuerys );
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************
// 2008-09-02

/*! @brief Erstellt eine neue Parameterdatenbank */

void MainWindow::doCreateParameterDB()
{
    int                i				    = 0;
    int                err				    = 0;
    int                i_stopProgress	    = 0;

    QFileInfo          fi_PDB( gs_FilenamePDB );

    QString            s_FilenameBackupPDB = fi_PDB.absolutePath() + "/" + tr( "ParameterDB_Backup.pdb" );
    QString            s_FileChecksum      = "";

    QStringList        sl_FilenameIn;

// **********************************************************************************************

    gi_NumOfParameterFiles = getNumParameterQuerys();

    if ( gi_NumOfParameterFiles > 0 )
    {
        if ( fi_PDB.exists() == false )
            QDir().mkdir( QDir::toNativeSeparators( fi_PDB.absolutePath() ) );
        else
            QFile::remove( gs_FilenamePDB );

// **********************************************************************************************

        initFileProgress( gi_NumOfParameterFiles, tr( "ParameterDB.pdb" ), tr( "Refreshing parameter database (download)..." ) );

        while ( ( i < gi_NumOfParameterFiles ) && ( err == _NOERROR_ ) && ( i_stopProgress != _APPBREAK_ ) )
        {
            QString s_FilenameParameterQuery = QString( "Parameter%1" ).arg( i+1 );
            QString s_FilenameParameter      = fi_PDB.absolutePath() + "/" + s_FilenameParameterQuery + ".txt";
            QString s_DDI_URL                = QLatin1String( "https://pangaea.de/ddi/xxx.tab?retr=parameter/" ) + s_FilenameParameterQuery + QLatin1String( ".retr&conf=parameter/ParameterListOrderByID.conf&format=textfile&charset=UTF-8" );

            downloadFile( findCurl(), s_DDI_URL, s_FilenameParameter );

            if ( ( i < gi_NumOfParameterFiles-1 ) && ( QFileInfo( s_FilenameParameter ).size() > 730000 ) )
                sl_FilenameIn.append( s_FilenameParameter );

            if ( i == gi_NumOfParameterFiles-1 )
                sl_FilenameIn.append( s_FilenameParameter );

            i_stopProgress = incFileProgress( gi_NumOfParameterFiles, ++i );
        }

        resetFileProgress( gi_NumOfParameterFiles );
    }

// **********************************************************************************************

    if ( ( gi_NumOfParameterFiles > 0 ) && ( sl_FilenameIn.count() > 0 ) && ( sl_FilenameIn.count() == gi_NumOfParameterFiles ) && ( err == _NOERROR_ ) && ( i_stopProgress != _APPBREAK_ ) )
    {
        err = concatenateFiles( gs_FilenamePDB, sl_FilenameIn, tr( "Refreshing parameter database (concatenate)..." ), 1, false );

        if ( err == _NOERROR_ )
        {
            s_FileChecksum = getFileChecksum( gs_FilenamePDB );

            if ( s_FileChecksum == gs_PDBChecksum )
                err = -145;

            gs_PDBChecksum          = s_FileChecksum;
            gi_NumOfParametersInPDB = 0;

            if ( err == _NOERROR_ )
            {
                QFile::remove( s_FilenameBackupPDB );
                QFile::copy( gs_FilenamePDB, s_FilenameBackupPDB );
            }
        }

        setStatusBar( tr( "Done" ), 2 );
    }
    else
    {
        if ( QFileInfo( s_FilenameBackupPDB ).exists() == true )
        {
            sl_FilenameIn.clear();
            sl_FilenameIn.append( s_FilenameBackupPDB );

            err = concatenateFiles( gs_FilenamePDB, sl_FilenameIn, tr( "Restoring parameter database..." ), 1, false );

            err = -143;

            setStatusBar( tr( "Refreshing parameter database was canceled, try again!" ), 2 );
        }
        else
        {
            err = -144;

            setStatusBar( tr( "Refreshing parameter database was canceled, try again!" ), 2 );
        }
    }

// **********************************************************************************************

    if ( gi_NumOfParameterFiles == 0 ) // https://store.pangaea.de/software/Split2Events/ParameterDDI.txt not readable
        err = -146;

// **********************************************************************************************

    onError( err );
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************
// 2008-09-02

/*! @brief Fuegt einer bestehenden Parameterdatenbank neue Parameter hinzu */

void MainWindow::doMergeParameterDB()
{
    int                i				    = 0;
    int                err				    = 0;
    int                i_stopProgress	    = 0;

    QFileInfo          fi_PDB( gs_FilenamePDB );

    QString            s_FilenameBackupPDB = fi_PDB.absolutePath() + "/" + tr( "ParameterDB_Backup.pdb" );
    QString            s_FileChecksum      = "";

    QStringList        sl_FilenameIn;

// **********************************************************************************************

    gi_NumOfParameterFiles = getNumParameterQuerys();

    if ( gi_NumOfParameterFiles > 0 )
    {
        if ( fi_PDB.exists() == false )
            QDir().mkdir( QDir::toNativeSeparators( fi_PDB.absolutePath() ) );
        else
            QFile::remove( gs_FilenamePDB );

// **********************************************************************************************

        initFileProgress( gi_NumOfParameterFiles, tr( "ParameterDB.pdb" ), tr( "Refreshing parameter database (download)..." ) );

        while ( ( i < gi_NumOfParameterFiles ) && ( err == _NOERROR_ ) && ( i_stopProgress != _APPBREAK_ ) )
        {
            QString s_FilenameParameterQuery = QString( "Parameter%1" ).arg( i+1 );
            QString s_FilenameParameter      = fi_PDB.absolutePath() + "/" + s_FilenameParameterQuery + ".txt";

            if ( i < gi_NumOfParameterFiles-1 )
            {
                QFile outfile( s_FilenameParameter );

                if ( outfile.exists() == false )
                    err = -142;
                else
                    sl_FilenameIn.append( s_FilenameParameter );
            }
            else
            {
                QFile outfile( s_FilenameParameter );

                if ( outfile.exists() == true )
                    outfile.remove();

                QString s_DDI_URL = QLatin1String( "https://pangaea.de/ddi/xxx.tab?retr=parameter/" ) + s_FilenameParameterQuery + QLatin1String( ".retr&conf=parameter/ParameterListOrderByID.conf&format=textfile&charset=UTF-8" );

                downloadFile( findCurl(), s_DDI_URL, s_FilenameParameter );

                if ( QFileInfo( s_FilenameParameter ).size() > 41 ) // Header contains 41 Bytes
                    sl_FilenameIn.append( s_FilenameParameter );
            }

            i_stopProgress = incFileProgress( gi_NumOfParameterFiles, ++i );
        }

        resetFileProgress( gi_NumOfParameterFiles );
    }

// **********************************************************************************************

    if ( ( sl_FilenameIn.count() > 0 ) && ( sl_FilenameIn.count() == gi_NumOfParameterFiles ) && ( err == _NOERROR_ ) && ( i_stopProgress != _APPBREAK_ ) )
    {
        err = concatenateFiles( gs_FilenamePDB, sl_FilenameIn, tr( "Refreshing parameter database (concatenate)..." ), 1, false );

        if ( err == _NOERROR_ )
        {
            s_FileChecksum = getFileChecksum( gs_FilenamePDB );

            if ( s_FileChecksum == gs_PDBChecksum )
                err = -145;

            gs_PDBChecksum          = s_FileChecksum;
            gi_NumOfParametersInPDB = 0;

            if ( err == _NOERROR_ )
            {
                QFile::remove( s_FilenameBackupPDB );
                QFile::copy( gs_FilenamePDB, s_FilenameBackupPDB );
            }
        }

        setStatusBar( tr( "Done" ), 2 );
    }
    else
    {
        if ( QFileInfo( s_FilenameBackupPDB ).exists() == true )
        {
            sl_FilenameIn.clear();
            sl_FilenameIn.append( s_FilenameBackupPDB );

            err = concatenateFiles( gs_FilenamePDB, sl_FilenameIn, tr( "Restoring parameter database..." ), 1, false );

            err = -143;

            setStatusBar( tr( "Refreshing parameter database was canceled, try again!" ), 2 );
        }
        else
        {
            err = -144;

            setStatusBar( tr( "Refreshing parameter database was canceled, try again!" ), 2 );
        }
    }

// **********************************************************************************************

    if ( gi_NumOfParameterFiles == 0 ) // https://store.pangaea.de/software/Split2Events/ParameterDDI.txt not readable
        err = -142;

// **********************************************************************************************

    onError( err );
}
